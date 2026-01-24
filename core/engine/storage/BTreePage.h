#pragma once

#include "Page.h"
#include "../../sql/ast/Value.h"
#include "Record.h"

namespace storage {

/**
 * BTreePage is a base class for B+ Tree index pages.
 * It is mapped directly onto the 8KB data of a Page.
 */
class BTreePage {
public:
    static constexpr uint32_t INVALID_PAGE_ID = 0;

    struct BTreeHeader {
        uint32_t parent_page_id;
        uint16_t size;
        uint16_t max_size;
    };

    BTreePage(char* data) : data_(data) {}

    PageType getPageType() const { return reinterpret_cast<const PageHeader*>(data_)->page_type; }
    void setPageType(PageType type) { reinterpret_cast<PageHeader*>(data_)->page_type = type; }

    uint32_t getParentPageId() const { return getBTreeHeader()->parent_page_id; }
    void setParentPageId(uint32_t id) { getBTreeHeader()->parent_page_id = id; }

    uint16_t getSize() const { return getBTreeHeader()->size; }
    void setSize(uint16_t size) { getBTreeHeader()->size = size; }

    uint16_t getMaxSize() const { return getBTreeHeader()->max_size; }
    void setMaxSize(uint16_t max) { getBTreeHeader()->max_size = max; }

    bool isRoot() const { return getParentPageId() == INVALID_PAGE_ID; }
    bool isLeaf() const { return getPageType() == PageType::BTREE_LEAF; }

protected:
    BTreeHeader* getBTreeHeader() { 
        return reinterpret_cast<BTreeHeader*>(data_ + Page::HEADER_SIZE); 
    }
    const BTreeHeader* getBTreeHeader() const { 
        return reinterpret_cast<const BTreeHeader*>(data_ + Page::HEADER_SIZE); 
    }

    char* data_;
};

/**
 * BTreeInternalPage stores (Key, PageID) pairs.
 * The first PageID (ptr0) corresponds to keys less than key1.
 */
class BTreeInternalPage : public BTreePage {
public:
    // Header location: PageHeader + BTreeHeader
    static constexpr size_t HEADER_SIZE = Page::HEADER_SIZE + sizeof(BTreePage::BTreeHeader);
    static constexpr size_t ENTRY_SIZE = 40; // Max key size 32 + PageID 4 + padding
    
    BTreeInternalPage(char* data) : BTreePage(data) {}

    void init(uint32_t parent_id = INVALID_PAGE_ID);

    uint32_t valueAt(int index) const;
    void setValueAt(int index, uint32_t value);

    Value keyAt(int index) const;
    void setKeyAt(int index, const Value& key);

    // Binary search for child page
    uint32_t lookup(const Value& key) const;
    
    // Helpers for insert/split
    void insert(const Value& key, uint32_t value);
    void moveHalfTo(BTreeInternalPage* recipient);
};

/**
 * BTreeLeafPage stores (Key, RID) pairs.
 */
class BTreeLeafPage : public BTreePage {
public:
    // Header includes next_page_id after BTreeHeader
    // Header includes next_page_id after BTreeHeader
    static constexpr size_t HEADER_SIZE = Page::HEADER_SIZE + sizeof(BTreePage::BTreeHeader) + sizeof(uint32_t); 
    static constexpr size_t ENTRY_SIZE = 40; // Max key size 32 + RID 6 + padding
    
    BTreeLeafPage(char* data) : BTreePage(data) {}

    void init(uint32_t parent_id = INVALID_PAGE_ID);

    uint32_t getNextPageId() const;
    void setNextPageId(uint32_t id);

    RID valueAt(int index) const;
    void setValueAt(int index, const RID& value);

    Value keyAt(int index) const;
    void setKeyAt(int index, const Value& key);

    int lookup(const Value& key) const;
    void insert(const Value& key, const RID& value);
    void moveHalfTo(BTreeLeafPage* recipient);
};

} // namespace storage
