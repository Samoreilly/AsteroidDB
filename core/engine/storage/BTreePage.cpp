#include "BTreePage.h"
#include <algorithm>
#include <cstring>

namespace storage {

// --- BTreeInternalPage ---

void BTreeInternalPage::init(uint32_t parent_id) {
    setPageType(PageType::BTREE_INTERNAL);
    setParentPageId(parent_id);
    setSize(0);
    // Entries are (Value, PageID). Value up to 32 bytes, PageID 4 bytes.
    // To keep it simple, we use a fixed space for value.
    setMaxSize((Page::PAGE_SIZE - HEADER_SIZE) / ENTRY_SIZE);
}

uint32_t BTreeInternalPage::valueAt(int index) const {
    return *reinterpret_cast<const uint32_t*>(data_ + HEADER_SIZE + index * ENTRY_SIZE + 32);
}

void BTreeInternalPage::setValueAt(int index, uint32_t value) {
    *reinterpret_cast<uint32_t*>(data_ + HEADER_SIZE + index * ENTRY_SIZE + 32) = value;
}

Value BTreeInternalPage::keyAt(int index) const {
    // Deserialize only the key part (32 bytes)
    // We assume the first character of the buffer tells the type based on Record::serialize
    std::vector<Value> vals = Record::deserialize(data_ + HEADER_SIZE + index * ENTRY_SIZE, 32);
    if (vals.empty()) return Value();
    return vals[0];
}

void BTreeInternalPage::setKeyAt(int index, const Value& key) {
    std::vector<char> serialized = Record::serialize({key});
    size_t size = std::min(serialized.size(), (size_t)32);
    std::memset(data_ + HEADER_SIZE + index * ENTRY_SIZE, 0, 32);
    std::memcpy(data_ + HEADER_SIZE + index * ENTRY_SIZE, serialized.data(), size);
}

uint32_t BTreeInternalPage::lookup(const Value& key) const {
    // Find the first index i such that keyAt(i) > key
    // The child at valueAt(i-1) contains keys in range [keyAt(i-1), keyAt(i))
    // We'll use the convention that valueAt(i) is the child for keys >= keyAt(i)
    int count = getSize();
    if (count == 0) return INVALID_PAGE_ID;

    int idx = 0;
    while (idx < count - 1 && keyAt(idx + 1) <= key) {
        idx++;
    }
    return valueAt(idx);
}

void BTreeInternalPage::insert(const Value& key, uint32_t value) {
    int index = getSize();
    while (index > 0 && keyAt(index - 1) > key) {
        std::memcpy(data_ + HEADER_SIZE + index * ENTRY_SIZE, data_ + HEADER_SIZE + (index - 1) * ENTRY_SIZE, ENTRY_SIZE);
        index--;
    }
    setKeyAt(index, key);
    setValueAt(index, value);
    setSize(getSize() + 1);
}

void BTreeInternalPage::moveHalfTo(BTreeInternalPage* recipient) {
    int half = getSize() / 2;
    int move_count = getSize() - half;
    std::memcpy(recipient->data_ + HEADER_SIZE, data_ + HEADER_SIZE + half * ENTRY_SIZE, move_count * ENTRY_SIZE);
    recipient->setSize(move_count);
    setSize(half);
}

// --- BTreeLeafPage ---

void BTreeLeafPage::init(uint32_t parent_id) {
    setPageType(PageType::BTREE_LEAF);
    setParentPageId(parent_id);
    setSize(0);
    setNextPageId(INVALID_PAGE_ID);
    setMaxSize((Page::PAGE_SIZE - HEADER_SIZE) / ENTRY_SIZE);
}

uint32_t BTreeLeafPage::getNextPageId() const {
    return *reinterpret_cast<const uint32_t*>(data_ + Page::HEADER_SIZE + sizeof(BTreePage::BTreeHeader));
}

void BTreeLeafPage::setNextPageId(uint32_t id) {
    *reinterpret_cast<uint32_t*>(data_ + Page::HEADER_SIZE + sizeof(BTreePage::BTreeHeader)) = id;
}

RID BTreeLeafPage::valueAt(int index) const {
    const char* ptr = data_ + HEADER_SIZE + index * ENTRY_SIZE + 32;
    uint32_t pid = *reinterpret_cast<const uint32_t*>(ptr);
    uint16_t sid = *reinterpret_cast<const uint16_t*>(ptr + 4);
    return RID(pid, sid);
}

void BTreeLeafPage::setValueAt(int index, const RID& value) {
    char* ptr = data_ + HEADER_SIZE + index * ENTRY_SIZE + 32;
    *reinterpret_cast<uint32_t*>(ptr) = value.page_id;
    *reinterpret_cast<uint16_t*>(ptr + 4) = value.slot_id;
}

Value BTreeLeafPage::keyAt(int index) const {
    std::vector<Value> vals = Record::deserialize(data_ + HEADER_SIZE + index * ENTRY_SIZE, 32);
    if (vals.empty()) return Value();
    return vals[0];
}

void BTreeLeafPage::setKeyAt(int index, const Value& key) {
    std::vector<char> serialized = Record::serialize({key});
    size_t size = std::min(serialized.size(), (size_t)32);
    std::memset(data_ + HEADER_SIZE + index * ENTRY_SIZE, 0, 32);
    std::memcpy(data_ + HEADER_SIZE + index * ENTRY_SIZE, serialized.data(), size);
}

int BTreeLeafPage::lookup(const Value& key) const {
    // Binary search for exact match
    int left = 0, right = getSize() - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        Value midKey = keyAt(mid);
        if (midKey == key) return mid;
        if (midKey < key) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

void BTreeLeafPage::insert(const Value& key, const RID& value) {
    int index = getSize();
    while (index > 0 && keyAt(index - 1) > key) {
        std::memcpy(data_ + HEADER_SIZE + index * ENTRY_SIZE, data_ + HEADER_SIZE + (index - 1) * ENTRY_SIZE, ENTRY_SIZE);
        index--;
    }
    setKeyAt(index, key);
    setValueAt(index, value);
    setSize(getSize() + 1);
}

void BTreeLeafPage::moveHalfTo(BTreeLeafPage* recipient) {
    int half = getSize() / 2;
    int move_count = getSize() - half;
    std::memcpy(recipient->data_ + HEADER_SIZE, data_ + HEADER_SIZE + half * ENTRY_SIZE, move_count * ENTRY_SIZE);
    recipient->setSize(move_count);
    setSize(half);
}

} // namespace storage
