#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <vector>

namespace storage {

// Page types
enum class PageType : uint8_t {
    INVALID_PAGE = 0,
    DATA_PAGE = 1,
    HEADER_PAGE = 2,
    FREE_PAGE = 3,
    BTREE_INTERNAL = 4,
    BTREE_LEAF = 5
};

// Slot structure for slotted page layout
struct Slot {
    uint16_t offset;  // Offset from start of page to record
    uint16_t length;  // Length of the record
    bool is_deleted;  // Whether this slot is deleted
    
    Slot() : offset(0), length(0), is_deleted(false) {}
    Slot(uint16_t off, uint16_t len) : offset(off), length(len), is_deleted(false) {}
};

// Page header structure
struct PageHeader {
    uint32_t page_id;           // Page ID
    PageType page_type;         // Type of page
    uint16_t free_space_pointer; // Points to start of free space
    uint16_t slot_count;        // Number of slots
    uint16_t free_space_size;   // Amount of free space
    
    PageHeader() : page_id(0), page_type(PageType::INVALID_PAGE), 
                   free_space_pointer(0), slot_count(0), free_space_size(0) {}
};

class Page {
public:
    static constexpr size_t PAGE_SIZE = 8192;  // 8KB pages
    static constexpr size_t HEADER_SIZE = sizeof(PageHeader);
    
private:
    char data_[PAGE_SIZE];
    bool is_dirty_;
    
    PageHeader* getHeader() {
        return reinterpret_cast<PageHeader*>(data_);
    }
    
    const PageHeader* getHeader() const {
        return reinterpret_cast<const PageHeader*>(data_);
    }
    
    Slot* getSlot(uint16_t slot_id) {
        // Slots grow from end of page backwards
        return reinterpret_cast<Slot*>(data_ + PAGE_SIZE - (slot_id + 1) * sizeof(Slot));
    }
    
    const Slot* getSlot(uint16_t slot_id) const {
        // Slots grow from end of page backwards
        return reinterpret_cast<const Slot*>(data_ + PAGE_SIZE - (slot_id + 1) * sizeof(Slot));
    }
    
public:
    Page();
    Page(uint32_t page_id, PageType page_type);
    
    // Initialize page
    void init(uint32_t page_id, PageType page_type);
    
    // Insert a record, returns slot_id or -1 on failure
    int insertRecord(const char* record_data, uint16_t record_size);
    
    // Delete a record by slot_id
    bool deleteRecord(uint16_t slot_id);
    
    // Get record data by slot_id
    const char* getRecord(uint16_t slot_id, uint16_t& out_size) const;
    
    // Update a record
    bool updateRecord(uint16_t slot_id, const char* record_data, uint16_t record_size);
    
    // Page metadata accessors
    uint32_t getPageId() const { return getHeader()->page_id; }
    PageType getPageType() const { return getHeader()->page_type; }
    uint16_t getSlotCount() const { return getHeader()->slot_count; }
    uint16_t getFreeSpace() const { return getHeader()->free_space_size; }
    
    // Dirty flag management
    bool isDirty() const { return is_dirty_; }
    void setDirty(bool dirty) { is_dirty_ = dirty; }
    
    // Get raw page data for I/O
    char* getData() { return data_; }
    const char* getData() const { return data_; }
    
    // Compact page to reclaim deleted space
    void compact();
    
private:
    // Calculate free space
    void updateFreeSpace();
};

} // namespace storage
