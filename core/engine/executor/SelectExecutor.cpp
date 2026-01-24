#include "SelectExecutor.h"
#include <iostream>
#include <iomanip>
#include <chrono>

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
    
    // Check for "Index Seek" opportunity (Equality on indexed column)
    storage::BPlusTree* index = catalog_->getIndex(stmt->table);
    if (index != nullptr && schema->indexColumn != -1 && stmt->whereClause != nullptr) {
        // Simple pattern matching for Index Seek: id = literal
        if (auto* binExpr = dynamic_cast<BinaryExpression*>(stmt->whereClause.get())) {
            if (binExpr->op == "=") {
                Value targetValue;
                bool indexMatch = false;
                
                // Case: id = 5
                if (auto* ident = dynamic_cast<Identifier*>(binExpr->left.get())) {
                    std::string indexColName = schema->columns[schema->indexColumn].name;
                    if (ident->token == indexColName) {
                        if (auto* lit = dynamic_cast<Literal*>(binExpr->right.get())) {
                            targetValue = lit->value;
                            indexMatch = true;
                        }
                    }
                }
                // Case: 5 = id
                else if (auto* ident = dynamic_cast<Identifier*>(binExpr->right.get())) {
                    std::string indexColName = schema->columns[schema->indexColumn].name;
                    if (ident->token == indexColName) {
                        if (auto* lit = dynamic_cast<Literal*>(binExpr->left.get())) {
                            targetValue = lit->value;
                            indexMatch = true;
                        }
                    }
                }
                
                if (indexMatch) {
                    storage::RID rid = index->getValue(targetValue);
                    if (rid.isValid()) {
                        std::vector<Value> recordValues = table->getRecord(rid);
                        
                        // Project selected columns
                        ResultRow row;
                        row.columnNames = selectedColumnNames;
                        for (int idx : selectedColumnIndices) {
                            if (idx < static_cast<int>(recordValues.size())) {
                                row.values.push_back(recordValues[idx]);
                            }
                        }
                        results.push_back(row);
                        rowCount = 1;

                        auto end = std::chrono::high_resolution_clock::now();
                        last_query_time_ms_ = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                        std::cout << "Index Seek used for " << schema->columns[schema->indexColumn].name << ". Selected 1 row(s)" << std::endl;
                        return results;
                    } else {
                        // Index lookup found no match, return 0 rows
                        auto end = std::chrono::high_resolution_clock::now();
                        last_query_time_ms_ = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                        std::cout << "Index Seek used for " << schema->columns[schema->indexColumn].name << ". Selected 0 row(s)" << std::endl;
                        return results;
                    }
                }
            }
        }
    }

    // Fallback: Full Scan
    for (auto it = table->begin(); it.isValid(); it.next()) {
        // Get full record
        std::vector<Value> recordValues = it.getRecord();
        
        // Build current row map for WHERE clause evaluation
        std::map<std::string, Value> currentRow;
        for (size_t i = 0; i < schema->columns.size() && i < recordValues.size(); i++) {
            currentRow[schema->columns[i].name] = recordValues[i];
        }
        executor_.setCurrentRow(currentRow);
        
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
