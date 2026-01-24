#include "TableHeap.h"
#include <stdexcept>
#include <filesystem>

namespace storage {

TableHeap::TableHeap(const std::string& table_name, const std::string& db_directory)
    : name_(table_name), first_page_id_(1), last_search_page_id_(1) {
    
    // Create database file path
    db_file_ = db_directory + "/" + table_name + ".db";
    
    // Initialize page manager and buffer pool
    page_manager_ = std::make_unique<PageManager>(db_file_);
    buffer_pool_ = std::make_unique<BufferPool>(page_manager_.get());
    
    // Check if table is new (only has header page)
    if (page_manager_->getPageCount() <= 1) {
        initialize();
    }
}

TableHeap::~TableHeap() {
    // Flush all pages before destruction
    if (buffer_pool_) {
        buffer_pool_->flushAll();
    }
}

void TableHeap::initialize() {
    // Create first data page
    uint32_t page_id;
    buffer_pool_->newPage(PageType::DATA_PAGE, page_id);
    first_page_id_ = page_id;
    last_search_page_id_ = page_id;
    buffer_pool_->unpinPage(page_id, true);
}

RID TableHeap::insertRecord(const std::vector<Value>& values) {
    // Serialize the record
    std::vector<char> serialized = Record::serialize(values);
    
    if (serialized.size() > Page::PAGE_SIZE - Page::HEADER_SIZE - sizeof(Slot)) {
        throw std::runtime_error("Record too large to fit in a page");
    }
    
    // Find a page with enough space
    uint32_t page_id = findPageWithSpace(serialized.size());
    
    // Get the page
    Page* page = buffer_pool_->getPage(page_id);
    
    // Insert record into page
    int slot_id = page->insertRecord(serialized.data(), static_cast<uint16_t>(serialized.size()));
    
    if (slot_id < 0) {
        buffer_pool_->unpinPage(page_id, false);
        throw std::runtime_error("Failed to insert record into page");
    }
    
    // Unpin page (mark as dirty)
    buffer_pool_->unpinPage(page_id, true);
    
    return RID(page_id, static_cast<uint16_t>(slot_id));
}

std::vector<Value> TableHeap::getRecord(const RID& rid) {
    if (!rid.isValid()) {
        throw std::runtime_error("Invalid RID");
    }
    
    // Get the page
    Page* page = buffer_pool_->getPage(rid.page_id);
    
    // Get record data
    uint16_t size;
    const char* data = page->getRecord(rid.slot_id, size);
    
    if (data == nullptr) {
        buffer_pool_->unpinPage(rid.page_id, false);
        throw std::runtime_error("Record not found or deleted");
    }
    
    // Deserialize
    std::vector<Value> values = Record::deserialize(data, size);
    
    // Unpin page
    buffer_pool_->unpinPage(rid.page_id, false);
    
    return values;
}

bool TableHeap::updateRecord(const RID& rid, const std::vector<Value>& values) {
    if (!rid.isValid()) {
        return false;
    }
    
    // Serialize new record
    std::vector<char> serialized = Record::serialize(values);
    
    // Get the page
    Page* page = buffer_pool_->getPage(rid.page_id);
    
    // Update record
    bool success = page->updateRecord(rid.slot_id, serialized.data(), 
                                     static_cast<uint16_t>(serialized.size()));
    
    // Unpin page
    buffer_pool_->unpinPage(rid.page_id, success);
    
    return success;
}

bool TableHeap::deleteRecord(const RID& rid) {
    if (!rid.isValid()) {
        return false;
    }
    
    // Get the page
    Page* page = buffer_pool_->getPage(rid.page_id);
    
    // Delete record
    bool success = page->deleteRecord(rid.slot_id);
    
    // Unpin page
    buffer_pool_->unpinPage(rid.page_id, success);
    
    return success;
}

uint32_t TableHeap::findPageWithSpace(size_t required_space) {
    // Simple strategy: iterate through existing pages
    uint32_t page_count = page_manager_->getPageCount();
    
    // Check if hint is valid
    if (last_search_page_id_ < first_page_id_ || last_search_page_id_ >= page_count) {
        last_search_page_id_ = first_page_id_;
    }

    // Start from hint
    for (uint32_t page_id = last_search_page_id_; page_id < page_count; page_id++) {
        Page* page = buffer_pool_->getPage(page_id);
        
        if (page->getPageType() == PageType::DATA_PAGE && 
            page->getFreeSpace() >= required_space + sizeof(Slot)) {
            buffer_pool_->unpinPage(page_id, false);
            last_search_page_id_ = page_id;
            return page_id;
        }
        
        buffer_pool_->unpinPage(page_id, false);
    }
    
    // No suitable page found, allocate new one
    uint32_t new_page_id;
    buffer_pool_->newPage(PageType::DATA_PAGE, new_page_id);
    buffer_pool_->unpinPage(new_page_id, true);
    
    last_search_page_id_ = new_page_id;
    return new_page_id;
}

TableHeap::Iterator TableHeap::begin() {
    return Iterator(this, first_page_id_, 0);
}

// Iterator implementation

TableHeap::Iterator::Iterator(TableHeap* table, uint32_t page_id, uint16_t slot_id)
    : table_(table), current_page_id_(page_id), current_slot_id_(slot_id), current_page_(nullptr) {
    
    if (page_id != 0) {
        loadPage(page_id);
        advance();
    }
}

bool TableHeap::Iterator::isValid() const {
    return current_page_ != nullptr && current_page_id_ != 0;
}

void TableHeap::Iterator::next() {
    if (!isValid()) {
        return;
    }
    
    current_slot_id_++;
    advance();
}

RID TableHeap::Iterator::getRID() const {
    return RID(current_page_id_, current_slot_id_);
}

std::vector<Value> TableHeap::Iterator::getRecord() {
    if (!isValid()) {
        throw std::runtime_error("Invalid iterator");
    }
    
    uint16_t size;
    const char* data = current_page_->getRecord(current_slot_id_, size);
    
    if (data == nullptr) {
        throw std::runtime_error("Failed to get record from iterator");
    }
    
    return Record::deserialize(data, size);
}

void TableHeap::Iterator::advance() {
    while (true) {
        if (current_page_ == nullptr) {
            // Check if we reached the end of the file
            if (current_page_id_ >= table_->page_manager_->getPageCount()) {
                current_page_id_ = 0;
                return;
            }

            // Try to load the current page
            loadPage(current_page_id_);
            
            if (current_page_ == nullptr) {
                // Not a data page (e.g., B+ Tree page), skip it
                current_page_id_++;
                continue;
            }
            
            // Successfully loaded a data page, start from the first slot
            current_slot_id_ = 0;
        }

        // We have a pinned data page, check current slot
        if (current_slot_id_ < current_page_->getSlotCount()) {
            uint16_t size;
            const char* data = current_page_->getRecord(current_slot_id_, size);
            if (data != nullptr) {
                // Found a valid record
                return;
            }
            // Slot is deleted, try next slot
            current_slot_id_++;
        } else {
            // No more slots in this page, move to next page
            table_->buffer_pool_->unpinPage(current_page_id_, false);
            current_page_ = nullptr;
            current_page_id_++;
        }
    }
}

void TableHeap::Iterator::loadPage(uint32_t page_id) {
    current_page_ = table_->buffer_pool_->getPage(page_id);
    current_page_id_ = page_id;
    
    // Check if it's a data page
    if (current_page_->getPageType() != PageType::DATA_PAGE) {
        table_->buffer_pool_->unpinPage(page_id, false);
        current_page_ = nullptr;
    }
}

} // namespace storage
