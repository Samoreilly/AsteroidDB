#include "Catalog.h"
#include <algorithm>
#include <fstream>
#include <iostream>

namespace executor {

int TableSchema::getColumnIndex(const std::string& columnName) const {
    for (size_t i = 0; i < columns.size(); i++) {
        if (columns[i].name == columnName) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool TableSchema::hasColumn(const std::string& columnName) const {
    return getColumnIndex(columnName) >= 0;
}

Catalog::Catalog(const std::string& db_directory) : db_directory_(db_directory) {
    load();
}

Catalog::~Catalog() {
    // Unique pointers will automatically clean up
}

bool Catalog::createTable(const std::string& tableName, const std::vector<ColumnInfo>& columns) {
    // Check if table already exists
    if (tableExists(tableName)) {
        return false;
    }
    
    // Create table schema
    TableSchema schema;
    schema.tableName = tableName;
    
    for (const auto& col : columns) {
        schema.columns.push_back(col);
    }
    
    // Automatically index the first column
    if (!schema.columns.empty()) {
        schema.indexColumn = 0;
    }
    
    // Create table heap
    auto tableHeap = std::make_unique<storage::TableHeap>(tableName, db_directory_);
    
    // Create B+ Tree index if specified
    if (schema.indexColumn != -1) {
        auto btree = std::make_unique<storage::BPlusTree>(tableName + "_idx", tableHeap->getBufferPool(), tableHeap->getPageManager());
        indices_[tableName] = std::move(btree);
        schema.indexRootPageId = indices_[tableName]->getRootPageId();
    }
    
    // Store in catalog
    schemas_[tableName] = schema;
    tables_[tableName] = std::move(tableHeap);
    
    save();
    return true;
}

bool Catalog::tableExists(const std::string& tableName) const {
    return schemas_.find(tableName) != schemas_.end();
}

storage::TableHeap* Catalog::getTable(const std::string& tableName) {
    auto it = tables_.find(tableName);
    if (it == tables_.end()) {
        return nullptr;
    }
    return it->second.get();
}

const TableSchema* Catalog::getSchema(const std::string& tableName) const {
    auto it = schemas_.find(tableName);
    if (it == schemas_.end()) {
        return nullptr;
    }
    return &it->second;
}

storage::BPlusTree* Catalog::getIndex(const std::string& tableName) {
    auto it = indices_.find(tableName);
    if (it == indices_.end()) {
        return nullptr;
    }
    return it->second.get();
}

bool Catalog::dropTable(const std::string& tableName) {
    if (!tableExists(tableName)) {
        return false;
    }
    
    tables_.erase(tableName);
    schemas_.erase(tableName);
    indices_.erase(tableName);
    
    save();
    return true;
}

void Catalog::save() {
    std::string path = db_directory_ + "/catalog.meta";
    std::ofstream out(path);
    if (!out.is_open()) return;
    
    out << schemas_.size() << "\n";
    for (const auto& [name, schema] : schemas_) {
        out << name << " " << schema.columns.size() << " " << schema.indexColumn << " " << schema.indexRootPageId << "\n";
        for (const auto& col : schema.columns) {
            out << col.name << " " << col.type << "\n";
        }
    }
}

void Catalog::load() {
    std::string path = db_directory_ + "/catalog.meta";
    std::ifstream in(path);
    if (!in.is_open()) return;
    
    size_t tableCount;
    if (!(in >> tableCount)) return;
    
    for (size_t i = 0; i < tableCount; i++) {
        std::string tableName;
        size_t colCount;
        int indexCol;
        uint32_t indexRoot;
        
        if (!(in >> tableName >> colCount >> indexCol >> indexRoot)) break;
        
        TableSchema schema;
        schema.tableName = tableName;
        schema.indexColumn = indexCol;
        schema.indexRootPageId = indexRoot;
        
        for (size_t j = 0; j < colCount; j++) {
            std::string colName, colType;
            if (!(in >> colName >> colType)) break;
            schema.columns.push_back(ColumnInfo(colName, colType));
        }
        
        schemas_[tableName] = schema;
        tables_[tableName] = std::make_unique<storage::TableHeap>(tableName, db_directory_);
        
        if (indexCol != -1) {
            auto btree = std::make_unique<storage::BPlusTree>(tableName + "_idx", tables_[tableName]->getBufferPool(), tables_[tableName]->getPageManager());
            btree->setRootPageId(indexRoot);
            indices_[tableName] = std::move(btree);
        }
    }
}

} // namespace executor
