#include "BPlusTree.h"
#include <iostream>

namespace storage {

BPlusTree::BPlusTree(const std::string& index_name, BufferPool& buffer_pool, PageManager& page_manager)
    : name_(index_name), buffer_pool_(buffer_pool), page_manager_(page_manager), root_page_id_(BTreePage::INVALID_PAGE_ID) {
}

RID BPlusTree::getValue(const Value& key) {
    if (root_page_id_ == BTreePage::INVALID_PAGE_ID) {
        return RID();
    }

    uint32_t leaf_id = findLeafPageId(key);
    Page* raw_page = buffer_pool_.getPage(leaf_id);
    BTreeLeafPage leaf(raw_page->getData());
    
    int index = leaf.lookup(key);
    RID result = (index != -1) ? leaf.valueAt(index) : RID();
    
    buffer_pool_.unpinPage(leaf_id, false);
    return result;
}

uint32_t BPlusTree::findLeafPageId(const Value& key) {
    uint32_t curr_id = root_page_id_;
    // std::cout << "BPlusTree: traversing from root " << curr_id << std::endl;
    
    while (true) {
        Page* raw_page = buffer_pool_.getPage(curr_id);
        BTreePage base(raw_page->getData());
        
        if (base.isLeaf()) {
            buffer_pool_.unpinPage(curr_id, false);
            return curr_id;
        }

        BTreeInternalPage internal(raw_page->getData());
        uint32_t next_id = internal.lookup(key);
        // std::cout << "BPlusTree: internal node " << curr_id << " pointing to " << next_id << std::endl;
        
        buffer_pool_.unpinPage(curr_id, false);
        curr_id = next_id;
        
        if (curr_id == 0 || curr_id > 1000000) {
             std::cerr << "BPlusTree: CRITICAL - invalid next page ID " << curr_id << " from internal node" << std::endl;
             return 0;
        }
    }
}

void BPlusTree::insert(const Value& key, const RID& rid) {
    if (root_page_id_ == BTreePage::INVALID_PAGE_ID) {
        // Create root leaf
        root_page_id_ = page_manager_.allocatePage(PageType::BTREE_LEAF);
        Page* raw_page = buffer_pool_.getPage(root_page_id_);
        BTreeLeafPage leaf(raw_page->getData());
        leaf.init(BTreePage::INVALID_PAGE_ID);
        leaf.insert(key, rid);
        buffer_pool_.unpinPage(root_page_id_, true);
        return;
    }

    uint32_t leaf_id = findLeafPageId(key);
    Page* raw_leaf = buffer_pool_.getPage(leaf_id);
    BTreeLeafPage leaf(raw_leaf->getData());

    leaf.insert(key, rid);

    if (leaf.getSize() >= leaf.getMaxSize()) {
        splitLeaf(&leaf, raw_leaf);
    } else {
        buffer_pool_.unpinPage(leaf_id, true);
    }
}

void BPlusTree::splitLeaf(BTreeLeafPage* leaf, Page* leaf_raw) {
    uint32_t old_leaf_id = leaf_raw->getPageId();
    uint32_t new_page_id = page_manager_.allocatePage(PageType::BTREE_LEAF);
    Page* raw_new = buffer_pool_.getPage(new_page_id);
    BTreeLeafPage new_leaf(raw_new->getData());
    
    new_leaf.init(leaf->getParentPageId());
    leaf->moveHalfTo(&new_leaf);
    
    new_leaf.setNextPageId(leaf->getNextPageId());
    leaf->setNextPageId(new_page_id);
    
    Value rising_key = new_leaf.keyAt(0);

    buffer_pool_.unpinPage(old_leaf_id, true);
    buffer_pool_.unpinPage(new_page_id, true);

    insertIntoParent(old_leaf_id, rising_key, new_page_id);
}

void BPlusTree::insertIntoParent(uint32_t old_page_id, const Value& key, uint32_t new_page_id) {
    Page* raw_old = buffer_pool_.getPage(old_page_id);
    BTreePage old_node(raw_old->getData());
    uint32_t parent_id = old_node.getParentPageId();
    buffer_pool_.unpinPage(old_page_id, false);

    if (parent_id == BTreePage::INVALID_PAGE_ID) {
        // Create new root
        uint32_t new_root_id = page_manager_.allocatePage(PageType::BTREE_INTERNAL);
        Page* raw_root = buffer_pool_.getPage(new_root_id);
        BTreeInternalPage root(raw_root->getData());
        root.init(BTreePage::INVALID_PAGE_ID);
        
        root.insert(Value(), old_page_id); 
        root.insert(key, new_page_id);
        
        root_page_id_ = new_root_id;
        
        // Update parents of children
        Page* c1 = buffer_pool_.getPage(old_page_id);
        BTreePage(c1->getData()).setParentPageId(new_root_id);
        buffer_pool_.unpinPage(old_page_id, true);
        
        Page* c2 = buffer_pool_.getPage(new_page_id);
        BTreePage(c2->getData()).setParentPageId(new_root_id);
        buffer_pool_.unpinPage(new_page_id, true);
        
        buffer_pool_.unpinPage(new_root_id, true);
        return;
    }

    Page* raw_parent = buffer_pool_.getPage(parent_id);
    BTreeInternalPage parent(raw_parent->getData());
    
    parent.insert(key, new_page_id);
    
    if (parent.getSize() >= parent.getMaxSize()) {
        splitInternal(&parent, raw_parent);
    } else {
        buffer_pool_.unpinPage(parent_id, true);
    }
}

void BPlusTree::splitInternal(BTreeInternalPage* internal, Page* internal_raw) {
    uint32_t old_id = internal_raw->getPageId();
    uint32_t new_page_id = page_manager_.allocatePage(PageType::BTREE_INTERNAL);
    Page* raw_new = buffer_pool_.getPage(new_page_id);
    BTreeInternalPage new_node(raw_new->getData());
    
    new_node.init(internal->getParentPageId());
    internal->moveHalfTo(&new_node);
    
    Value rising_key = new_node.keyAt(0);

    // Update children's parent pointers
    for (int i = 0; i < new_node.getSize(); i++) {
        uint32_t child_id = new_node.valueAt(i);
        Page* child = buffer_pool_.getPage(child_id);
        BTreePage(child->getData()).setParentPageId(new_page_id);
        buffer_pool_.unpinPage(child_id, true);
    }

    buffer_pool_.unpinPage(old_id, true);
    buffer_pool_.unpinPage(new_page_id, true);

    insertIntoParent(old_id, rising_key, new_page_id);
}

} // namespace storage
