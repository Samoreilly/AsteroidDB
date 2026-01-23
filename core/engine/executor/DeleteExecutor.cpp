#include "DeleteExecutor.h"
#include <iostream>
#include <vector>

namespace executor {

DeleteExecutor::DeleteExecutor(Catalog* catalog) : catalog_(catalog) {
}

void DeleteExecutor::execute(DeleteStatement* stmt) {
    if (stmt == nullptr) {
        throw std::runtime_error("DELETE statement is null");
    }
    
    // Get table
    storage::TableHeap* table = catalog_->getTable(stmt->table);
    if (table == nullptr) {
        std::cout << "Table '" << stmt->table << "' does not exist" << std::endl;
        return;
    }
    
    // Get schema
    const TableSchema* schema = catalog_->getSchema(stmt->table);
    
    // Collect RIDs to delete (can't delete while iterating)
    std::vector<storage::RID> toDelete;
    
    // Scan table
    for (auto it = table->begin(); it.isValid(); it.next()) {
        // Get record values
        std::vector<Value> recordValues = it.getRecord();
        
        // Build current row map
        std::map<std::string, Value> currentRow;
        for (size_t i = 0; i < schema->columns.size() && i < recordValues.size(); i++) {
            currentRow[schema->columns[i].name] = recordValues[i];
        }
        executor_.setCurrentRow(currentRow);
        
        // For DELETE, if no WHERE clause, delete all rows
        // Note: DeleteStatement in Node.h doesn't have whereClause field
        // So we'll delete all rows for now
        toDelete.push_back(it.getRID());
    }
    
    // Delete collected records
    int deletedCount = 0;
    for (const auto& rid : toDelete) {
        if (table->deleteRecord(rid)) {
            deletedCount++;
        }
    }
    
    std::cout << "Deleted " << deletedCount << " row(s)" << std::endl;
}

} // namespace executor
