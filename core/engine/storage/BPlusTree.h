#pragma once

#include "BufferPool.h"
#include "BTreePage.h"
#include "PageManager.h"
#include <string>

namespace storage {

class BPlusTree {
public:
    BPlusTree(const std::string& index_name, BufferPool& buffer_pool, PageManager& page_manager);

    // Get RID for a specific key
    RID getValue(const Value& key);

    // Insert a key-RID pair
    void insert(const Value& key, const RID& rid);

    // Get root page ID
    uint32_t getRootPageId() const { return root_page_id_; }
    void setRootPageId(uint32_t id) { root_page_id_ = id; }

    // Iterator for range scans
    class Iterator {
    public:
        Iterator(BufferPool& buffer_pool, uint32_t page_id, int index);
        ~Iterator();

        // Move constructor
        Iterator(Iterator&& other) noexcept;
        Iterator& operator=(Iterator&& other) noexcept;

        // No copy
        Iterator(const Iterator&) = delete;
        Iterator& operator=(const Iterator&) = delete;

        bool isEnd() const;
        void next();
        RID getRID() const;
        Value getKey() const;

    private:
        BufferPool& buffer_pool_;
        Page* curr_page_;
        uint32_t curr_page_id_;
        int curr_index_;
    };

    // Get iterator starting at specific key (or first key >= k)
    Iterator begin(const Value& key);
    
    // Get iterator starting at beginning
    Iterator begin();

private:
    // Returns pinned leaf page
    Page* findLeafPage(const Value& key);
    
    // Returns pinned leaf page (leftmost)
    Page* findFirstLeafPage();
    
    // Split logic
    void splitLeaf(BTreeLeafPage* leaf, Page* leaf_raw);
    void splitInternal(BTreeInternalPage* internal, Page* internal_raw);
    
    // Insert into parent
    void insertIntoParent(uint32_t old_page_id, const Value& key, uint32_t new_page_id);

    std::string name_;
    BufferPool& buffer_pool_;
    PageManager& page_manager_;
    uint32_t root_page_id_;
};

} // namespace storage
