#include "PageManager.h"
#include <iostream>
#include <cstring>

namespace storage {

PageManager::PageManager(const std::string& db_filename) 
    : filename_(db_filename), page_count_(0) {
    
    // Try to open existing file
    file_.open(filename_, std::ios::in | std::ios::out | std::ios::binary);
    
    if (!file_.is_open()) {
        // File doesn't exist, create new one
        initializeFile();
    } else {
        // File exists, read page count and free list
        file_.seekg(0, std::ios::end);
        std::streampos file_size = file_.tellg();
        page_count_ = static_cast<uint32_t>(file_size / Page::PAGE_SIZE);
        
        loadFreeList();
    }
}

PageManager::~PageManager() {
    if (file_.is_open()) {
        saveFreeList();
        flush();
        file_.close();
    }
}

bool PageManager::ensureFileOpen() {
    if (!file_.is_open()) {
        file_.open(filename_, std::ios::in | std::ios::out | std::ios::binary);
    }
    return file_.is_open();
}

void PageManager::initializeFile() {
    // Create new file
    file_.open(filename_, std::ios::out | std::ios::binary);
    file_.close();
    
    // Reopen for read/write
    file_.open(filename_, std::ios::in | std::ios::out | std::ios::binary);
    
    if (!file_.is_open()) {
        throw std::runtime_error("Failed to create database file: " + filename_);
    }
    
    // Create header page (page 0)
    Page header_page(0, PageType::HEADER_PAGE);
    writePage(header_page);
    page_count_ = 1;
}

uint32_t PageManager::allocatePage(PageType page_type) {
    if (!ensureFileOpen()) {
        throw std::runtime_error("Database file not open");
    }
    
    uint32_t page_id;
    
    // Try to reuse a free page
    if (!free_pages_.empty()) {
        auto it = free_pages_.begin();
        page_id = *it;
        free_pages_.erase(it);
    } else {
        // Allocate new page at end of file
        page_id = page_count_;
        page_count_++;
    }
    
    // Initialize the page
    Page page(page_id, page_type);
    writePage(page);
    
    return page_id;
}

void PageManager::deallocatePage(uint32_t page_id) {
    if (page_id == 0) {
        // Cannot deallocate header page
        return;
    }
    
    if (page_id < page_count_) {
        free_pages_.insert(page_id);
    }
}

bool PageManager::readPage(uint32_t page_id, Page& page) {
    if (!ensureFileOpen()) {
        return false;
    }
    
    if (page_id >= page_count_) {
        return false;
    }
    
    // Seek to page position
    std::streampos pos = static_cast<std::streampos>(page_id) * Page::PAGE_SIZE;
    file_.seekg(pos);
    
    if (!file_.good()) {
        return false;
    }
    
    // Read page data
    file_.read(page.getData(), Page::PAGE_SIZE);
    
    if (!file_.good()) {
        return false;
    }
    
    page.setDirty(false);
    return true;
}

bool PageManager::writePage(const Page& page) {
    if (!ensureFileOpen()) {
        return false;
    }
    
    uint32_t page_id = page.getPageId();
    
    // Seek to page position
    std::streampos pos = static_cast<std::streampos>(page_id) * Page::PAGE_SIZE;
    file_.seekp(pos);
    
    if (!file_.good()) {
        return false;
    }
    
    // Write page data
    file_.write(page.getData(), Page::PAGE_SIZE);
    
    if (!file_.good()) {
        return false;
    }
    
    return true;
}

void PageManager::flush() {
    if (file_.is_open()) {
        file_.flush();
    }
}

void PageManager::loadFreeList() {
    if (page_count_ == 0) {
        return;
    }
    
    // Read header page
    Page header_page;
    if (!readPage(0, header_page)) {
        return;
    }
    
    // Free list is stored in header page data area
    // Format: uint32_t count, followed by page_ids
    uint16_t size;
    const char* data = header_page.getRecord(0, size);
    
    if (data == nullptr || size < sizeof(uint32_t)) {
        return;
    }
    
    uint32_t free_count;
    std::memcpy(&free_count, data, sizeof(uint32_t));
    
    size_t offset = sizeof(uint32_t);
    for (uint32_t i = 0; i < free_count && offset + sizeof(uint32_t) <= size; i++) {
        uint32_t page_id;
        std::memcpy(&page_id, data + offset, sizeof(uint32_t));
        free_pages_.insert(page_id);
        offset += sizeof(uint32_t);
    }
}

void PageManager::saveFreeList() {
    if (page_count_ == 0) {
        return;
    }
    
    // Read header page
    Page header_page;
    if (!readPage(0, header_page)) {
        return;
    }
    
    // Serialize free list
    std::vector<char> free_list_data;
    uint32_t free_count = static_cast<uint32_t>(free_pages_.size());
    
    const char* count_bytes = reinterpret_cast<const char*>(&free_count);
    free_list_data.insert(free_list_data.end(), count_bytes, count_bytes + sizeof(uint32_t));
    
    for (uint32_t page_id : free_pages_) {
        const char* id_bytes = reinterpret_cast<const char*>(&page_id);
        free_list_data.insert(free_list_data.end(), id_bytes, id_bytes + sizeof(uint32_t));
    }
    
    // Store in header page
    // First delete existing record if any
    if (header_page.getSlotCount() > 0) {
        header_page.deleteRecord(0);
    }
    
    // Insert new free list
    header_page.insertRecord(free_list_data.data(), static_cast<uint16_t>(free_list_data.size()));
    
    // Write back
    writePage(header_page);
}

} // namespace storage
