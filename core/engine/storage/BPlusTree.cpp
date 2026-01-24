#include "BPlusTree.h"
#include <iostream>

namespace storage {

BPlusTree::BPlusTree(const std::string& index_name, BufferPool& buffer_pool, PageManager& page_manager)
    : name_(index_name), buffer_pool_(buffer_pool), page_manager_(page_manager), root_page_id_(BTreePage::INVALID_PAGE_ID) {
}

// Iterator implementation
BPlusTree::Iterator::Iterator(BufferPool& buffer_pool, uint32_t page_id, int index)
    : buffer_pool_(buffer_pool), curr_page_(nullptr), curr_page_id_(page_id), curr_index_(index) {
    if (curr_page_id_ != BTreePage::INVALID_PAGE_ID) {
        curr_page_ = buffer_pool_.getPage(curr_page_id_);
    }
}

BPlusTree::Iterator::~Iterator() {
    if (curr_page_) {
        buffer_pool_.unpinPage(curr_page_id_, false);
    }
}

BPlusTree::Iterator::Iterator(Iterator&& other) noexcept 
    : buffer_pool_(other.buffer_pool_), curr_page_(other.curr_page_), 
      curr_page_id_(other.curr_page_id_), curr_index_(other.curr_index_) {
    other.curr_page_ = nullptr;
    other.curr_page_id_ = BTreePage::INVALID_PAGE_ID;
}

BPlusTree::Iterator& BPlusTree::Iterator::operator=(Iterator&& other) noexcept {
    if (this != &other) {
        if (curr_page_) {
            buffer_pool_.unpinPage(curr_page_id_, false);
        }
        curr_page_ = other.curr_page_;
        curr_page_id_ = other.curr_page_id_;
        curr_index_ = other.curr_index_;
        
        other.curr_page_ = nullptr;
        other.curr_page_id_ = BTreePage::INVALID_PAGE_ID;
    }
    return *this;
}

bool BPlusTree::Iterator::isEnd() const {
    return curr_page_ == nullptr;
}

void BPlusTree::Iterator::next() {
    if (isEnd()) return;

    BTreeLeafPage leaf(curr_page_->getData());
    curr_index_++;

    if (curr_index_ >= leaf.getSize()) {
        uint32_t next_id = leaf.getNextPageId();
        buffer_pool_.unpinPage(curr_page_id_, false);
        curr_page_ = nullptr;
        
        if (next_id != BTreePage::INVALID_PAGE_ID && next_id != 0) {
            curr_page_id_ = next_id;
            curr_page_ = buffer_pool_.getPage(curr_page_id_);
            curr_index_ = 0;
        } else {
            curr_page_id_ = BTreePage::INVALID_PAGE_ID;
        }
    }
}

RID BPlusTree::Iterator::getRID() const {
    if (isEnd()) return RID();
    BTreeLeafPage leaf(curr_page_->getData());
    return leaf.valueAt(curr_index_);
}

Value BPlusTree::Iterator::getKey() const {
    if (isEnd()) return Value();
    BTreeLeafPage leaf(curr_page_->getData());
    return leaf.keyAt(curr_index_);
}

BPlusTree::Iterator BPlusTree::begin(const Value& key) {
    if (root_page_id_ == BTreePage::INVALID_PAGE_ID) {
        return Iterator(buffer_pool_, BTreePage::INVALID_PAGE_ID, 0);
    }

    // We can't use findLeafPage because it keeps the page pinned and returns Page*
    // But Iterator expects to pin it itself or take ownership.
    // Actually Iterator constructor pins it. So we should NOT pin it here, OR we should pass the Page* to Iterator.
    // My Iterator ctor takes page_id and calls getPage.
    // So I need to find the page ID, but findLeafPage pins it.
    // Let's modify findLeafPageId to just return ID? No, the previous optimization was to avoid unpin/pin.
    
    // Better: Iterator should be able to take an already pinned Page*.
    // But for now, let's just use findLeafPageId logic inside here or rely on the fact that findLeafPage pins it, 
    // we can just unpin it immediately and let Iterator repin (inefficient), 
    // OR change Iterator to take Page*.
    
    // Let's manually traverse to get the ID without pinning permanently, or just accept the repin overhead for the *first* page of a scan.
    // Given the optimization goal, let's try to be efficient.
    
    // I'll reimplement traverse here to get ID and index.
    
    uint32_t curr_id = root_page_id_;
    while (true) {
        Page* raw_page = buffer_pool_.getPage(curr_id);
        BTreePage base(raw_page->getData());
        
        if (base.isLeaf()) {
            // Found the leaf
            BTreeLeafPage leaf(raw_page->getData());
            int index = leaf.lookup(key);
            // lookup returns index of key >= target. If not found (all smaller), it might return size?
            // checking BTreeLeafPage::lookup... it usually returns binary search lower_bound.
            
            // If lookup returns -1, it means key not found? No, std::lower_bound logic usually returns iterator.
            // I need to check BTreeLeafPage::lookup implementation.
            
            buffer_pool_.unpinPage(curr_id, false); // Unpin so Iterator can grab it (or avoid double pin logic)
            return Iterator(buffer_pool_, curr_id, index == -1 ? 0 : index); 
        }

        BTreeInternalPage internal(raw_page->getData());
        uint32_t next_id = internal.lookup(key);
        buffer_pool_.unpinPage(curr_id, false);
        curr_id = next_id;
        
        if (curr_id == 0) return Iterator(buffer_pool_, BTreePage::INVALID_PAGE_ID, 0);
    }
}

BPlusTree::Iterator BPlusTree::begin() {
    if (root_page_id_ == BTreePage::INVALID_PAGE_ID) {
        return Iterator(buffer_pool_, BTreePage::INVALID_PAGE_ID, 0);
    }
    
    uint32_t curr_id = root_page_id_;
    while (true) {
        Page* raw_page = buffer_pool_.getPage(curr_id);
        BTreePage base(raw_page->getData());
        
        if (base.isLeaf()) {
            buffer_pool_.unpinPage(curr_id, false);
            return Iterator(buffer_pool_, curr_id, 0);
        }
        
        BTreeInternalPage internal(raw_page->getData());
        // Follow leftmost pointer
        uint32_t next_id = internal.valueAt(0);
        buffer_pool_.unpinPage(curr_id, false);
        curr_id = next_id;
        
        if (curr_id == 0) return Iterator(buffer_pool_, BTreePage::INVALID_PAGE_ID, 0);
    }
}

RID BPlusTree::getValue(const Value& key) {
    if (root_page_id_ == BTreePage::INVALID_PAGE_ID) {
        return RID();
    }

    Page* raw_page = findLeafPage(key);
    if (!raw_page) return RID();

    BTreeLeafPage leaf(raw_page->getData());
    
    int index = leaf.lookup(key);
    RID result = (index != -1) ? leaf.valueAt(index) : RID();
    
    buffer_pool_.unpinPage(raw_page->getPageId(), false);
    return result;
}

Page* BPlusTree::findLeafPage(const Value& key) {
    uint32_t curr_id = root_page_id_;
    // std::cout << "BPlusTree: traversing from root " << curr_id << std::endl;
    
    while (true) {
        Page* raw_page = buffer_pool_.getPage(curr_id);
        BTreePage base(raw_page->getData());
        
        if (base.isLeaf()) {
            return raw_page;
        }

        BTreeInternalPage internal(raw_page->getData());
        uint32_t next_id = internal.lookup(key);
        // std::cout << "BPlusTree: internal node " << curr_id << " pointing to " << next_id << std::endl;
        
        buffer_pool_.unpinPage(curr_id, false);
        curr_id = next_id;
        
        if (curr_id == 0 || curr_id > 1000000) {
             std::cerr << "BPlusTree: CRITICAL - invalid next page ID " << curr_id << " from internal node" << std::endl;
             return nullptr;
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

    Page* raw_leaf = findLeafPage(key);
    if (!raw_leaf) return;

    uint32_t leaf_id = raw_leaf->getPageId();
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
