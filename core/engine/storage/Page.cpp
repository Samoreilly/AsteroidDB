#include "Page.h"
#include <algorithm>
#include <stdexcept>

namespace storage {

Page::Page() : is_dirty_(false) {
    std::memset(data_, 0, PAGE_SIZE);
}

Page::Page(uint32_t page_id, PageType page_type) : is_dirty_(false) {
    init(page_id, page_type);
}

void Page::init(uint32_t page_id, PageType page_type) {
    std::memset(data_, 0, PAGE_SIZE);
    
    PageHeader* header = getHeader();
    header->page_id = page_id;
    header->page_type = page_type;
    header->free_space_pointer = HEADER_SIZE;
    header->slot_count = 0;
    header->free_space_size = PAGE_SIZE - HEADER_SIZE;
    
    is_dirty_ = true;
}

int Page::insertRecord(const char* record_data, uint16_t record_size) {
    if (record_data == nullptr || record_size == 0) {
        return -1;
    }
    
    PageHeader* header = getHeader();
    
    // Check if we have enough space for the record + slot
    uint16_t space_needed = record_size + sizeof(Slot);
    if (header->free_space_size < space_needed) {
        return -1;
    }
    
    // Try to find a deleted slot to reuse
    int slot_id = -1;
    for (uint16_t i = 0; i < header->slot_count; i++) {
        Slot* slot = getSlot(i);
        if (slot->is_deleted) {
            slot_id = i;
            break;
        }
    }
    
    // If no deleted slot, allocate a new one
    if (slot_id == -1) {
        slot_id = header->slot_count;
        header->slot_count++;
    }
    
    // Calculate offset for new record (grows from beginning)
    uint16_t record_offset = header->free_space_pointer;
    
    // Write record data
    std::memcpy(data_ + record_offset, record_data, record_size);
    
    // Update slot
    Slot* slot = getSlot(slot_id);
    slot->offset = record_offset;
    slot->length = record_size;
    slot->is_deleted = false;
    
    // Update header
    header->free_space_pointer += record_size;
    updateFreeSpace();
    
    is_dirty_ = true;
    return slot_id;
}

bool Page::deleteRecord(uint16_t slot_id) {
    PageHeader* header = getHeader();
    
    if (slot_id >= header->slot_count) {
        return false;
    }
    
    Slot* slot = getSlot(slot_id);
    if (slot->is_deleted) {
        return false;
    }
    
    slot->is_deleted = true;
    updateFreeSpace();
    
    is_dirty_ = true;
    return true;
}

const char* Page::getRecord(uint16_t slot_id, uint16_t& out_size) const {
    const PageHeader* header = getHeader();
    
    if (slot_id >= header->slot_count) {
        out_size = 0;
        return nullptr;
    }
    
    const Slot* slot = getSlot(slot_id);
    if (slot->is_deleted) {
        out_size = 0;
        return nullptr;
    }
    
    out_size = slot->length;
    return data_ + slot->offset;
}

bool Page::updateRecord(uint16_t slot_id, const char* record_data, uint16_t record_size) {
    PageHeader* header = getHeader();
    
    if (slot_id >= header->slot_count) {
        return false;
    }
    
    Slot* slot = getSlot(slot_id);
    if (slot->is_deleted) {
        return false;
    }
    
    // Simple case: new record fits in old space
    if (record_size <= slot->length) {
        std::memcpy(data_ + slot->offset, record_data, record_size);
        slot->length = record_size;
        is_dirty_ = true;
        return true;
    }
    
    // Complex case: delete and reinsert
    deleteRecord(slot_id);
    compact();
    
    // Try to insert
    uint16_t space_needed = record_size;
    if (header->free_space_size < space_needed) {
        return false;
    }
    
    uint16_t record_offset = header->free_space_pointer;
    std::memcpy(data_ + record_offset, record_data, record_size);
    
    slot->offset = record_offset;
    slot->length = record_size;
    slot->is_deleted = false;
    
    header->free_space_pointer += record_size;
    updateFreeSpace();
    
    is_dirty_ = true;
    return true;
}

void Page::compact() {
    PageHeader* header = getHeader();
    
    if (header->slot_count == 0) {
        return;
    }
    
    // Create temporary buffer for active records
    char temp_buffer[PAGE_SIZE];
    uint16_t write_offset = HEADER_SIZE;
    
    // Copy active records to temp buffer and update slot offsets
    for (uint16_t i = 0; i < header->slot_count; i++) {
        Slot* slot = getSlot(i);
        if (!slot->is_deleted) {
            std::memcpy(temp_buffer + write_offset, data_ + slot->offset, slot->length);
            slot->offset = write_offset;
            write_offset += slot->length;
        }
    }
    
    // Copy compacted data back
    std::memcpy(data_ + HEADER_SIZE, temp_buffer + HEADER_SIZE, write_offset - HEADER_SIZE);
    
    // Update free space pointer
    header->free_space_pointer = write_offset;
    updateFreeSpace();
    
    is_dirty_ = true;
}

void Page::updateFreeSpace() {
    PageHeader* header = getHeader();
    
    // Records grow from beginning (after header)
    uint16_t records_end = header->free_space_pointer;
    
    // Slots grow from end of page backwards
    uint16_t slots_start = PAGE_SIZE - header->slot_count * sizeof(Slot);
    
    if (slots_start >= records_end) {
        header->free_space_size = slots_start - records_end;
    } else {
        header->free_space_size = 0;
    }
}

} // namespace storage
