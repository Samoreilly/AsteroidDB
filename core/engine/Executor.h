#pragma once

#include <string>
#include <vector>
#include "../sql/ast/Value.h"
#include <map>

class Executor {

    std::map<std::string, Value> currentRow;

public:

    void setCurrentRow(const std::map<std::string, Value>& row) {
        currentRow = row;
    }

    const std::map<std::string, Value>& getCurrentRow() {
        return currentRow;
    }



};
