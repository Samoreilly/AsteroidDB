#include "Node.h"
#include "Expression.h"
#include <iostream>

Node::~Node() {}

SelectStatement::~SelectStatement() = default;

void SelectStatement::print() const {
    std::cout << "SelectStatement {" << std::endl;
    std::cout << "  columns: [";
    for (size_t i = 0; i < columns.size(); i++) {
        std::cout << columns[i];
        if (i < columns.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    std::cout << "  table: " << table << std::endl;
    std::cout << "  whereClause: ";
    if (whereClause) {
        std::cout << std::endl;
        whereClause->print(2);  // Pass indent level
    } else {
        std::cout << "null" << std::endl;
    }
    std::cout << "}" << std::endl;
}

CreateStatement::~CreateStatement() = default;

void CreateStatement::print() const {
    std::cout << "CreateStatement {" << std::endl;
    if (!database.empty()) {
        std::cout << "  database: " << database << std::endl;
    }
    std::cout << "  table: " << table << std::endl;
    std::cout << "  columns: [";

    for (size_t i = 0; i < columns.size(); i++) {
        std::cout << columns[i].name << " " << columns[i].type;
        if (i < columns.size() - 1) std::cout << ", ";
    }
    
    std::cout << "]" << std::endl;
    std::cout << "}" << std::endl;
}

