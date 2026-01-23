#pragma once

#include "../../sql/ast/Value.h"
#include <vector>
#include <cstdint>

namespace storage {

// Record ID (RID) uniquely identifies a record
struct RID {
    uint32_t page_id;
    uint16_t slot_id;
    
    RID() : page_id(0), slot_id(0) {}
    RID(uint32_t pid, uint16_t sid) : page_id(pid), slot_id(sid) {}
    
    bool operator==(const RID& other) const {
        return page_id == other.page_id && slot_id == other.slot_id;
    }
    
    bool isValid() const {
        return page_id != 0 || slot_id != 0;
    }
};

// Record class handles serialization/deserialization of records
class Record {
public:
    // Serialize a vector of Values into a byte array
    static std::vector<char> serialize(const std::vector<Value>& values);
    
    // Deserialize a byte array back into a vector of Values
    static std::vector<Value> deserialize(const char* data, size_t size);
    
    // Get the serialized size of a record
    static size_t getSerializedSize(const std::vector<Value>& values);
    
private:
    // Type tags for serialization
    enum class TypeTag : uint8_t {
        TYPE_NULL = 0,
        TYPE_INT = 1,
        TYPE_DOUBLE = 2,
        TYPE_STRING = 3,
        TYPE_BOOL = 4
    };
    
    static TypeTag getTypeTag(const Value& value);
};

} // namespace storage
