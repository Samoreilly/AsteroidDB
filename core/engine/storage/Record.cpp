#include "Record.h"
#include <cstring>
#include <stdexcept>

namespace storage {

Record::TypeTag Record::getTypeTag(const Value& value) {
    if (value.isNull()) return TypeTag::TYPE_NULL;
    if (value.isInt()) return TypeTag::TYPE_INT;
    if (value.isDouble()) return TypeTag::TYPE_DOUBLE;
    if (value.isString()) return TypeTag::TYPE_STRING;
    if (value.isBool()) return TypeTag::TYPE_BOOL;
    return TypeTag::TYPE_NULL;
}

std::vector<char> Record::serialize(const std::vector<Value>& values) {
    std::vector<char> buffer;
    buffer.reserve(getSerializedSize(values));
    
    // Write field count (2 bytes)
    uint16_t field_count = static_cast<uint16_t>(values.size());
    buffer.push_back(static_cast<char>(field_count & 0xFF));
    buffer.push_back(static_cast<char>((field_count >> 8) & 0xFF));
    
    // Write each field
    for (const auto& value : values) {
        TypeTag tag = getTypeTag(value);
        buffer.push_back(static_cast<char>(tag));
        
        switch (tag) {
            case TypeTag::TYPE_NULL:
                // No data for null
                break;
                
            case TypeTag::TYPE_INT: {
                int val = value.asInt();
                const char* bytes = reinterpret_cast<const char*>(&val);
                buffer.insert(buffer.end(), bytes, bytes + sizeof(int));
                break;
            }
            
            case TypeTag::TYPE_DOUBLE: {
                double val = value.asDouble();
                const char* bytes = reinterpret_cast<const char*>(&val);
                buffer.insert(buffer.end(), bytes, bytes + sizeof(double));
                break;
            }
            
            case TypeTag::TYPE_STRING: {
                std::string str = value.asString();
                uint16_t len = static_cast<uint16_t>(str.length());
                
                // Write length (2 bytes)
                buffer.push_back(static_cast<char>(len & 0xFF));
                buffer.push_back(static_cast<char>((len >> 8) & 0xFF));
                
                // Write string data
                buffer.insert(buffer.end(), str.begin(), str.end());
                break;
            }
            
            case TypeTag::TYPE_BOOL: {
                bool val = value.asBool();
                buffer.push_back(val ? 1 : 0);
                break;
            }
        }
    }
    
    return buffer;
}

std::vector<Value> Record::deserialize(const char* data, size_t size) {
    std::vector<Value> values;
    
    if (size < 2) {
        throw std::runtime_error("Invalid record: too small");
    }
    
    // Read field count
    uint16_t field_count = static_cast<uint8_t>(data[0]) | 
                          (static_cast<uint8_t>(data[1]) << 8);
    
    size_t offset = 2;
    
    for (uint16_t i = 0; i < field_count; i++) {
        if (offset >= size) {
            throw std::runtime_error("Invalid record: unexpected end");
        }
        
        TypeTag tag = static_cast<TypeTag>(static_cast<uint8_t>(data[offset++]));
        
        switch (tag) {
            case TypeTag::TYPE_NULL:
                values.push_back(Value());
                break;
                
            case TypeTag::TYPE_INT: {
                if (offset + sizeof(int) > size) {
                    throw std::runtime_error("Invalid record: truncated int");
                }
                int val;
                std::memcpy(&val, data + offset, sizeof(int));
                values.push_back(Value(val));
                offset += sizeof(int);
                break;
            }
            
            case TypeTag::TYPE_DOUBLE: {
                if (offset + sizeof(double) > size) {
                    throw std::runtime_error("Invalid record: truncated double");
                }
                double val;
                std::memcpy(&val, data + offset, sizeof(double));
                values.push_back(Value(val));
                offset += sizeof(double);
                break;
            }
            
            case TypeTag::TYPE_STRING: {
                if (offset + 2 > size) {
                    throw std::runtime_error("Invalid record: truncated string length");
                }
                
                uint16_t len = static_cast<uint8_t>(data[offset]) | 
                              (static_cast<uint8_t>(data[offset + 1]) << 8);
                offset += 2;
                
                if (offset + len > size) {
                    throw std::runtime_error("Invalid record: truncated string data");
                }
                
                std::string str(data + offset, len);
                values.push_back(Value(str));
                offset += len;
                break;
            }
            
            case TypeTag::TYPE_BOOL: {
                if (offset >= size) {
                    throw std::runtime_error("Invalid record: truncated bool");
                }
                bool val = (data[offset++] != 0);
                values.push_back(Value(val));
                break;
            }
            
            default:
                throw std::runtime_error("Invalid record: unknown type tag");
        }
    }
    
    return values;
}

size_t Record::getSerializedSize(const std::vector<Value>& values) {
    size_t size = 2; // field count
    
    for (const auto& value : values) {
        size += 1; // type tag
        
        if (value.isInt()) {
            size += sizeof(int);
        } else if (value.isDouble()) {
            size += sizeof(double);
        } else if (value.isString()) {
            size += 2 + value.asString().length(); // length + data
        } else if (value.isBool()) {
            size += 1;
        }
        // null has no data
    }
    
    return size;
}

} // namespace storage
