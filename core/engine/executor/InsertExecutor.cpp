#include "InsertExecutor.h"
#include <iostream>

namespace executor {

InsertExecutor::InsertExecutor(Catalog* catalog) : catalog_(catalog) {
}

void InsertExecutor::execute(InsertStatement* stmt) {
    if (stmt == nullptr) {
        throw std::runtime_error("INSERT statement is null");
    }
    
    // Get table
    storage::TableHeap* table = catalog_->getTable(stmt->table);
    if (table == nullptr) {
        std::cout << "Table '" << stmt->table << "' does not exist" << std::endl;
        return;
    }
    
    // Get schema
    const TableSchema* schema = catalog_->getSchema(stmt->table);
    
    // Evaluate expressions to get values
    std::vector<Value> allValues;
    for (const auto& expr : stmt->inputs) {
        Value val = expr->eval(&executor_);
        allValues.push_back(val);
    }
    
    size_t numColumns = stmt->columns.empty() ? schema->columns.size() : stmt->columns.size();
    
    if (allValues.empty() || allValues.size() % numColumns != 0) {
        std::cout << "Value count (" << allValues.size() 
                  << ") is not a multiple of column count (" << numColumns << ")" << std::endl;
        return;
    }
    
    size_t numRows = allValues.size() / numColumns;
    int successCount = 0;
    
    for (size_t r = 0; r < numRows; ++r) {
        std::vector<Value> values;
        for (size_t c = 0; c < numColumns; ++c) {
            values.push_back(allValues[r * numColumns + c]);
        }
        
        // Reorder values to match schema order if specific columns were provided
        if (!stmt->columns.empty()) {
            std::vector<Value> orderedValues(schema->columns.size());
            for (size_t i = 0; i < stmt->columns.size(); i++) {
                int colIndex = schema->getColumnIndex(stmt->columns[i]);
                if (colIndex < 0) {
                    std::cout << "Column '" << stmt->columns[i] << "' does not exist in table" << std::endl;
                    return;
                }
                orderedValues[colIndex] = values[i];
            }
            values = orderedValues;
        }
        
        // Insert into table
        try {
            storage::RID rid = table->insertRecord(values);
            successCount++;
        } catch (const std::exception& e) {
            std::cout << "Insert failed for row " << (r + 1) << ": " << e.what() << std::endl;
        }
    }
    
    if (numRows > 0) {
        std::cout << "Inserted " << successCount << " record(s)" << std::endl;
    }
}

} // namespace executor
