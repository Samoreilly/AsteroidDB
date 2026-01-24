#pragma once

#include "Page.h"
#include "Record.h"
#include "BufferPool.h"
#include "PageManager.h"
#include <string>
#include <memory>
#include <vector>

namespace storage {

class TableHeap {
public:
    TableHeap(const std::string& table_name, const std::string& db_directory = ".");
    ~TableHeap();
    
    // Insert a record, returns RID
    RID insertRecord(const std::vector<Value>& values);
    
    // Get a record by RID
    std::vector<Value> getRecord(const RID& rid);
    
    // Update a record
    bool updateRecord(const RID& rid, const std::vector<Value>& values);
    
    // Delete a record
    bool deleteRecord(const RID& rid);
    
    // Table scan iterator
    class Iterator {
    public:
        Iterator(TableHeap* table, uint32_t page_id, uint16_t slot_id);
        
        bool isValid() const;
        void next();
        RID getRID() const;
        std::vector<Value> getRecord();
        
    private:
        TableHeap* table_;
        uint32_t current_page_id_;
        uint16_t current_slot_id_;
        Page* current_page_;
        
        void advance();
        void loadPage(uint32_t page_id);
    };
    
    // Begin a table scan
    Iterator begin();
    
    // Get table name
    const std::string& getName() const { return name_; }

    // Component access for index management
    BufferPool& getBufferPool() { return *buffer_pool_; }
    PageManager& getPageManager() { return *page_manager_; }
    
private:
    std::string name_;
    std::string db_file_;
    std::unique_ptr<PageManager> page_manager_;
    std::unique_ptr<BufferPool> buffer_pool_;
    uint32_t first_page_id_;
    
    // Find a page with enough free space
    uint32_t findPageWithSpace(size_t required_space);
    
    // Initialize table (create first data page)
    void initialize();

    uint32_t last_search_page_id_;
};

} // namespace storage
