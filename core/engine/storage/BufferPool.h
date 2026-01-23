#pragma once

#include "Page.h"
#include "PageManager.h"
#include <unordered_map>
#include <list>
#include <memory>

namespace storage {

// Buffer pool entry
struct BufferPoolFrame {
    Page page;
    uint32_t page_id;
    int pin_count;
    bool is_dirty;
    
    BufferPoolFrame() : page_id(0), pin_count(0), is_dirty(false) {}
};

class BufferPool {
public:
    static constexpr size_t DEFAULT_POOL_SIZE = 128; // 128 pages = 1MB
    
    BufferPool(PageManager* page_manager, size_t pool_size = DEFAULT_POOL_SIZE);
    ~BufferPool();
    
    // Get a page (pins it automatically)
    Page* getPage(uint32_t page_id);
    
    // Create a new page
    Page* newPage(PageType page_type, uint32_t& out_page_id);
    
    // Pin a page (increment reference count)
    bool pinPage(uint32_t page_id);
    
    // Unpin a page (decrement reference count)
    bool unpinPage(uint32_t page_id, bool is_dirty = false);
    
    // Flush a specific page to disk
    bool flushPage(uint32_t page_id);
    
    // Flush all dirty pages to disk
    void flushAll();
    
    // Delete a page
    bool deletePage(uint32_t page_id);
    
private:
    PageManager* page_manager_;
    size_t pool_size_;
    
    // LRU list: most recently used at front
    std::list<uint32_t> lru_list_;
    
    // Map from page_id to frame and LRU iterator
    std::unordered_map<uint32_t, std::pair<BufferPoolFrame*, std::list<uint32_t>::iterator>> page_table_;
    
    // Pool of frames
    std::vector<std::unique_ptr<BufferPoolFrame>> frames_;
    
    // Find a victim page to evict
    BufferPoolFrame* findVictim();
    
    // Evict a page from the buffer pool
    bool evictPage(BufferPoolFrame* frame);
    
    // Update LRU for a page access
    void updateLRU(uint32_t page_id);
};

} // namespace storage
