#pragma once

#include "Page.h"
#include <string>
#include <fstream>
#include <vector>
#include <memory>
#include <unordered_set>

namespace storage {

class PageManager {
public:
    PageManager(const std::string& db_filename);
    ~PageManager();
    
    // Allocate a new page and return its page_id
    uint32_t allocatePage(PageType page_type);
    
    // Deallocate a page (add to free list)
    void deallocatePage(uint32_t page_id);
    
    // Read a page from disk into the provided Page object
    bool readPage(uint32_t page_id, Page& page);
    
    // Write a page to disk
    bool writePage(const Page& page);
    
    // Flush all pending writes
    void flush();
    
    // Get total number of pages
    uint32_t getPageCount() const { return page_count_; }
    
private:
    std::string filename_;
    std::fstream file_;
    uint32_t page_count_;
    std::unordered_set<uint32_t> free_pages_;
    
    // Ensure file is open
    bool ensureFileOpen();
    
    // Initialize a new database file
    void initializeFile();
    
    // Load free page list from header page
    void loadFreeList();
    
    // Save free page list to header page
    void saveFreeList();
};

} // namespace storage
