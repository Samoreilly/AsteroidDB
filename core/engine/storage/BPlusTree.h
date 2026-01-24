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

private:
    uint32_t findLeafPageId(const Value& key);
    
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
