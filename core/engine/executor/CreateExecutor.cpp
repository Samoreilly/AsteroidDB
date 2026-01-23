#include "CreateExecutor.h"
#include <iostream>

namespace executor {

CreateExecutor::CreateExecutor(Catalog* catalog) : catalog_(catalog) {
}

void CreateExecutor::execute(CreateStatement* stmt) {
    if (stmt == nullptr) {
        throw std::runtime_error("CREATE statement is null");
    }
    
    // Check if table already exists
    if (catalog_->tableExists(stmt->table)) {
        std::cout << "Table '" << stmt->table << "' already exists" << std::endl;
        return;
    }
    
    // Extract column information
    std::vector<ColumnInfo> columns;
    for (const auto& col : stmt->columns) {
        columns.push_back(ColumnInfo(col.name, col.type));
    }
    
    // Create table in catalog
    if (catalog_->createTable(stmt->table, columns)) {
        std::cout << "Table '" << stmt->table << "' created successfully with " 
                  << columns.size() << " columns" << std::endl;
    } else {
        std::cout << "Failed to create table '" << stmt->table << "'" << std::endl;
    }
}

} // namespace executor
