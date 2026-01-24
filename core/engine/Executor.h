#pragma once

#include <string>
#include <vector>
#include "../sql/ast/Value.h"
#include "executor/Catalog.h"
#include <map>
#include <stdexcept>

class Executor {

    std::map<std::string, Value> currentRow;
    const std::vector<Value>* currentValues = nullptr;
    const executor::TableSchema* currentSchema = nullptr;

public:

    void setCurrentRow(const std::map<std::string, Value>& row) {
        currentRow = row;
        currentValues = nullptr;
        currentSchema = nullptr;
    }

    void setCurrentRow(const std::vector<Value>& values, const executor::TableSchema& schema) {
        currentValues = &values;
        currentSchema = &schema;
    }

    const std::map<std::string, Value>& getCurrentRow() {
        return currentRow;
    }

    Value getValue(const std::string& name) {
        if (currentValues && currentSchema) {
            int idx = currentSchema->getColumnIndex(name);
            if (idx >= 0 && idx < static_cast<int>(currentValues->size())) {
                return (*currentValues)[idx];
            }
        }
        if (currentRow.count(name)) {
            return currentRow.at(name);
        }
        throw std::runtime_error("Column not found: " + name);
    }

};
