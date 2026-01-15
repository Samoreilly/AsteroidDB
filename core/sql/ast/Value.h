#include <string>
#include <variant>
#include <iostream>

class Value {

    std::variant<int, double, std::string, bool, std::monostate> data;


public:

    Value() : data(std::monostate{}) {}
    Value(int i) : data(i) {}
    Value(double d) : data(d) {}
    Value(std::string s) : data(s) {}
    Value(bool b) : data(b) {}
    Value(const char* s) : data(std::string(s)) {}


    bool isInt() const { return std::holds_alternative<int>(data); }
    bool isDouble() const { return std::holds_alternative<double>(data); }
    bool isString() const { return std::holds_alternative<std::string>(data); }
    bool isBool() const { return std::holds_alternative<bool>(data); }
    bool isNull() const { return std::holds_alternative<std::monostate>(data); }


    int asInt() const { return std::get<int>(data); }
    double asDouble() const { return std::get<double>(data); }
    std::string asString() const { return std::get<std::string>(data); }
    bool asBool() const { return std::get<bool>(data); }
    
    bool operator>(const Value& other) const {
        if (isInt() && other.isInt()) {
            return asInt() > other.asInt();
        }
        if (isDouble() && other.isDouble()) {
            return asDouble() > other.asDouble();
        }
        if (isString() && other.isString()) {
            return asString() > other.asString();
        }
        throw std::runtime_error("Cannot compare these types");
    }
    
    bool operator==(const Value& other) const {
        if (isNull() || other.isNull()) return false;
        
        if (isInt() && other.isInt()) return asInt() == other.asInt();
        if (isString() && other.isString()) return asString() == other.asString();
        if (isBool() && other.isBool()) return asBool() == other.asBool();
        
        return false;
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Value& v) {
        if (v.isNull()) os << "NULL";
        else if (v.isInt()) os << v.asInt();
        else if (v.isDouble()) os << v.asDouble();
        else if (v.isString()) os << "'" << v.asString() << "'";
        else if (v.isBool()) os << (v.asBool() ? "true" : "false");
        return os;
    }
};
