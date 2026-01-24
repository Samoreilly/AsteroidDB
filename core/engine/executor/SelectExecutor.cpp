#include "SelectExecutor.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <climits>

namespace executor {

SelectExecutor::SelectExecutor(Catalog* catalog) : catalog_(catalog), last_query_time_ms_(0) {
}

bool SelectExecutor::evaluateWhere(Expression* whereClause) {
    if (whereClause == nullptr) {
        return true;  // No WHERE clause means all rows match
    }
    
    Value result = whereClause->eval(&executor_);
    return result.isBool() && result.asBool();
}

// Helper to find index start condition
void findStartIndex(Expression* expr, const std::string& colName, Value& outStartKey, bool& outHasStart, bool& outInclusive) {
    if (!expr) return;

    if (auto* bin = dynamic_cast<BinaryExpression*>(expr)) {
        if (bin->op == "and") {
            findStartIndex(bin->left.get(), colName, outStartKey, outHasStart, outInclusive);
            findStartIndex(bin->right.get(), colName, outStartKey, outHasStart, outInclusive);
            // In a real optimizer we would merge ranges (e.g. > 5 AND > 10 -> > 10). 
            // Here simpler logic: last one wins or we trust traversal order.
            return;
        }

        // Check for col op literal
        std::string op = bin->op;
        Expression* left = bin->left.get();
        Expression* right = bin->right.get();
        
        Identifier* ident = dynamic_cast<Identifier*>(left);
        Literal* lit = dynamic_cast<Literal*>(right);
        
        if (!ident) {
            // Swap check: literal op col
            ident = dynamic_cast<Identifier*>(right);
            lit = dynamic_cast<Literal*>(left);
            // Flip operator direction for canonical form
            if (op == ">") op = "<";
            else if (op == "<") op = ">";
            else if (op == ">=") op = "<=";
            else if (op == "<=") op = ">=";
        }

        if (ident && lit && ident->token == colName) {
            // We have: colName op literal
            if (op == "=" || op == ">=" || op == ">") {
                // Potential start key
                // For > val, we can start at val. Filtering will skip the equal part if needed.
                // Actually BPlusTree::begin(val) finds first >= val.
                // If op is >, finding >= val is correct start.
                
                // If we already have a start key, we should pick the stricter one (max).
                // But simplifying: just take this one.
                outStartKey = lit->value;
                outHasStart = true;
                outInclusive = (op != ">");
            }
        }
    }
}

std::vector<ResultRow> SelectExecutor::execute(SelectStatement* stmt) {
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<ResultRow> results;
    int rowCount = 0;
    
    if (stmt == nullptr) {
        throw std::runtime_error("SELECT statement is null");
    }
    
    // Get table
    storage::TableHeap* table = catalog_->getTable(stmt->table);
    if (table == nullptr) {
        std::cout << "Table '" << stmt->table << "' does not exist" << std::endl;
        return results;
    }
    
    // Get schema
    const TableSchema* schema = catalog_->getSchema(stmt->table);
    
    // Determine which columns to select
    std::vector<int> selectedColumnIndices;
    std::vector<std::string> selectedColumnNames;
    
    if (stmt->columns.size() == 1 && stmt->columns[0] == "*") {
        // SELECT * - all columns
        for (size_t i = 0; i < schema->columns.size(); i++) {
            selectedColumnIndices.push_back(i);
            selectedColumnNames.push_back(schema->columns[i].name);
        }
    } else {
        // Specific columns
        for (const auto& colName : stmt->columns) {
            int idx = schema->getColumnIndex(colName);
            if (idx < 0) {
                std::cout << "Column '" << colName << "' does not exist in table" << std::endl;
                return results;
            }
            selectedColumnIndices.push_back(idx);
            selectedColumnNames.push_back(colName);
        }
    }
    
    // Check for Index Scan opportunity
    storage::BPlusTree* index = catalog_->getIndex(stmt->table);
    bool indexScan = false;
    Value startKey;
    bool hasStartKey = false;
    bool startInclusive = true;

    if (index != nullptr && schema->indexColumn != -1 && stmt->whereClause != nullptr) {
        findStartIndex(stmt->whereClause.get(), schema->columns[schema->indexColumn].name, startKey, hasStartKey, startInclusive);
        
        // If found a start key, use index scan
        if (hasStartKey) {
            indexScan = true;
        }
    }

    if (indexScan) {
        // Index Scan
        auto it = index->begin(startKey);
        
        while (!it.isEnd()) {
            storage::RID rid = it.getRID();
            
            // Optimization: If we have the key in index, we could check key predicates BEFORE fetching record.
            // But SelectExecutor infrastructure requires evaluating full expression which might depend on other cols.
            // So we fetch the record.
            
            try {
                std::vector<Value> recordValues = table->getRecord(rid);
                
                // Set context using vector optimization
                executor_.setCurrentRow(recordValues, *schema);
                
                if (evaluateWhere(stmt->whereClause.get())) {
                    ResultRow row;
                    row.columnNames = selectedColumnNames;
                    for (int idx : selectedColumnIndices) {
                        if (idx < static_cast<int>(recordValues.size())) {
                            row.values.push_back(recordValues[idx]);
                        }
                    }
                    results.push_back(row);
                    rowCount++;
                }
            } catch (...) {
                // Record might be deleted or invalid
            }
            
            it.next();
        }
        std::cout << "Index Scan used for " << schema->columns[schema->indexColumn].name << ". ";
    } else {
        // Full Scan
        for (auto it = table->begin(); it.isValid(); it.next()) {
            // Get full record
            std::vector<Value> recordValues = it.getRecord();
            
            // Use optimized context
            executor_.setCurrentRow(recordValues, *schema);
            
            // Evaluate WHERE clause
            if (!evaluateWhere(stmt->whereClause.get())) {
                continue;
            }
            
            // Project selected columns
            ResultRow row;
            row.columnNames = selectedColumnNames;
            for (int idx : selectedColumnIndices) {
                if (idx < static_cast<int>(recordValues.size())) {
                    row.values.push_back(recordValues[idx]);
                }
            }
            
            results.push_back(row);
            rowCount++;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    last_query_time_ms_ = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "Selected " << rowCount << " row(s)" << std::endl;
    
    return results;
}

void SelectExecutor::printResults(const std::vector<ResultRow>& results) {
    if (results.empty()) {
        std::cout << "No results (0 rows, " << last_query_time_ms_ << " ms)" << std::endl;
        return;
    }
    
    // Print header
    std::cout << "\n";
    for (const auto& colName : results[0].columnNames) {
        std::cout << std::setw(15) << std::left << colName;
    }
    std::cout << "\n";
    
    // Print separator
    for (size_t i = 0; i < results[0].columnNames.size(); i++) {
        std::cout << std::string(15, '-');
    }
    std::cout << "\n";
    
    // Print rows
    for (const auto& row : results) {
        for (const auto& val : row.values) {
            std::cout << std::setw(15) << std::left << val;
        }
        std::cout << "\n";
    }
    std::cout << "\n(" << results.size() << " rows, " << last_query_time_ms_ << " ms)" << std::endl;
}

} // namespace executor
