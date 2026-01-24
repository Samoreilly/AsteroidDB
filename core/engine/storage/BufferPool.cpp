#include "BufferPool.h"
#include <iostream>
#include <stdexcept>

namespace storage {

BufferPool::BufferPool(PageManager* page_manager, size_t pool_size)
    : page_manager_(page_manager), pool_size_(pool_size) {
    
    if (page_manager == nullptr) {
        throw std::runtime_error("PageManager cannot be null");
    }
    
    // Initialize frames
    frames_.reserve(pool_size);
    for (size_t i = 0; i < pool_size; i++) {
        frames_.push_back(std::make_unique<BufferPoolFrame>());
    }
}

BufferPool::~BufferPool() {
    flushAll();
}

Page* BufferPool::getPage(uint32_t page_id) {
    // Check if page is already in buffer pool
    auto it = page_table_.find(page_id);
    if (it != page_table_.end()) {
        BufferPoolFrame* frame = it->second.first;
        frame->pin_count++;
        updateLRU(page_id);
        return &frame->page;
    }
    
    // Page not in pool, need to fetch from disk
    BufferPoolFrame* frame = findVictim();
    if (frame == nullptr) {
        throw std::runtime_error("No available frames in buffer pool");
    }
    
    // Evict current page if present
    if (frame->page_id != 0) {
        evictPage(frame);
    }
    
    // Read page from disk
    if (!page_manager_->readPage(page_id, frame->page)) {
        throw std::runtime_error("Failed to read page from disk");
    }
    
    // Setup frame
    frame->page_id = page_id;
    frame->pin_count = 1;
    frame->is_dirty = false;
    
    // Add to LRU list
    lru_list_.push_front(page_id);
    page_table_[page_id] = {frame, lru_list_.begin()};
    
    return &frame->page;
}

Page* BufferPool::newPage(PageType page_type, uint32_t& out_page_id) {
    // Allocate new page from page manager
    out_page_id = page_manager_->allocatePage(page_type);
    
    // Find a frame for it
    BufferPoolFrame* frame = findVictim();
    if (frame == nullptr) {
        throw std::runtime_error("No available frames in buffer pool");
    }
    
    // Evict current page if present
    if (frame->page_id != 0) {
        evictPage(frame);
    }
    
    // Initialize new page in frame
    frame->page.init(out_page_id, page_type);
    frame->page_id = out_page_id;
    frame->pin_count = 1;
    frame->is_dirty = true;
    
    // Add to LRU list
    lru_list_.push_front(out_page_id);
    page_table_[out_page_id] = {frame, lru_list_.begin()};
    
    return &frame->page;
}

bool BufferPool::pinPage(uint32_t page_id) {
    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        return false;
    }
    
    it->second.first->pin_count++;
    updateLRU(page_id);
    return true;
}

bool BufferPool::unpinPage(uint32_t page_id, bool is_dirty) {
    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        return false;
    }
    
    BufferPoolFrame* frame = it->second.first;
    
    if (frame->pin_count <= 0) {
        return false;
    }
    
    frame->pin_count--;
    
    if (is_dirty) {
        frame->is_dirty = true;
        frame->page.setDirty(true);
    }
    
    return true;
}

bool BufferPool::flushPage(uint32_t page_id) {
    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        return false;
    }
    
    BufferPoolFrame* frame = it->second.first;
    
    if (frame->is_dirty || frame->page.isDirty()) {
        if (!page_manager_->writePage(frame->page)) {
            return false;
        }
        frame->is_dirty = false;
        frame->page.setDirty(false);
    }
    
    return true;
}

void BufferPool::flushAll() {
    for (const auto& entry : page_table_) {
        flushPage(entry.first);
    }
    page_manager_->flush();
}

bool BufferPool::deletePage(uint32_t page_id) {
    // Remove from buffer pool if present
    auto it = page_table_.find(page_id);
    if (it != page_table_.end()) {
        BufferPoolFrame* frame = it->second.first;
        
        if (frame->pin_count > 0) {
            return false; // Cannot delete pinned page
        }
        
        // Remove from LRU list
        lru_list_.erase(it->second.second);
        
        // Clear frame
        frame->page_id = 0;
        frame->is_dirty = false;
        frame->pin_count = 0;
        
        page_table_.erase(it);
    }
    
    // Mark as free in page manager
    page_manager_->deallocatePage(page_id);
    
    return true;
}

BufferPoolFrame* BufferPool::findVictim() {
    // First, try to find an empty frame
    for (auto& frame : frames_) {
        if (frame->page_id == 0) {
            return frame.get();
        }
    }
    
    // Use LRU: find unpinned page from end of list
    for (auto it = lru_list_.rbegin(); it != lru_list_.rend(); ++it) {
        uint32_t page_id = *it;
        auto table_it = page_table_.find(page_id);
        if (table_it != page_table_.end()) {
            BufferPoolFrame* frame = table_it->second.first;
            if (frame->pin_count == 0) {
                return frame;
            }
        }
    }
    
    // No unpinned pages available
    return nullptr;
}

bool BufferPool::evictPage(BufferPoolFrame* frame) {
    if (frame->page_id == 0) {
        return true; // Already empty
    }
    
    // Write back if dirty
    if (frame->is_dirty || frame->page.isDirty()) {
        if (!page_manager_->writePage(frame->page)) {
            return false;
        }
    }
    
    // Remove from page table and LRU list
    auto it = page_table_.find(frame->page_id);
    if (it != page_table_.end()) {
        lru_list_.erase(it->second.second);
        page_table_.erase(it);
    }
    
    frame->page_id = 0;
    frame->is_dirty = false;
    frame->pin_count = 0;
    
    return true;
}

void BufferPool::updateLRU(uint32_t page_id) {
    auto it = page_table_.find(page_id);
    if (it == page_table_.end()) {
        return;
    }
    
    // Move to front of LRU list using splice (no allocation)
    lru_list_.splice(lru_list_.begin(), lru_list_, it->second.second);
}

} // namespace storage
