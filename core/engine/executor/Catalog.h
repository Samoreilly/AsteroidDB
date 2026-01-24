#pragma once

#include "../storage/TableHeap.h"
#include <string>
#include <map>
#include <memory>
#include <vector>
#include "../storage/BPlusTree.h"

namespace executor {

// Column information in schema
struct ColumnInfo {
    std::string name;
    std::string type;  // INT, VARCHAR, DOUBLE, BOOL
    
    ColumnInfo() = default;
    ColumnInfo(const std::string& n, const std::string& t) : name(n), type(t) {}
};

// Table schema
struct TableSchema {
    std::string tableName;
    std::vector<ColumnInfo> columns;
    
    // Get column index by name
    int getColumnIndex(const std::string& columnName) const;
    
    // Check if column exists
    bool hasColumn(const std::string& columnName) const;
    
    // Index information
    int indexColumn = -1; // -1 if no index
    uint32_t indexRootPageId = 0;
};

// Catalog manages all tables and their schemas
class Catalog {
public:
    Catalog(const std::string& db_directory = ".");
    ~Catalog();
    
    // Create a new table
    bool createTable(const std::string& tableName, const std::vector<ColumnInfo>& columns);
    
    // Check if table exists
    bool tableExists(const std::string& tableName) const;
    
    // Get table heap
    storage::TableHeap* getTable(const std::string& tableName);
    
    // Get table schema
    const TableSchema* getSchema(const std::string& tableName) const;

    // Get table index
    storage::BPlusTree* getIndex(const std::string& tableName);
    
    // Drop table
    bool dropTable(const std::string& tableName);
    
    void save();
    
private:
    std::string db_directory_;
    
    // Table name -> TableHeap
    std::map<std::string, std::unique_ptr<storage::TableHeap>> tables_;
    
    // Table name -> Schema
    std::map<std::string, TableSchema> schemas_;
    
    // Table name -> BPlusTree
    std::map<std::string, std::unique_ptr<storage::BPlusTree>> indices_;
    
    void load();
};

} // namespace executor
