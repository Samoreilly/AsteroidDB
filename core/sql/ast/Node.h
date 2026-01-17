#pragma once

#include <iostream>
#include <vector>

#include "../lexer/TokenDef.h"
#include <memory>

class Expression;

class Node {
public:

    virtual ~Node();
    virtual void exec() = 0;
};

class SelectStatement : public Node {
public:

    //select certain fields from a table
    std::vector<std::string> columns;
    std::string table;
    std::unique_ptr<Expression> whereClause;

    void exec() override {
        std::cout << "Executing select from TABLE: " << table << "\n";
    }
    
    void print() const {
        std::cout << "SelectStatement {" << std::endl;
        std::cout << "  columns: [";
        for (size_t i = 0; i < columns.size(); i++) {
            std::cout << columns[i];
            if (i < columns.size() - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
        std::cout << "  table: " << table << std::endl;
        std::cout << "  whereClause: " << (whereClause ? "present" : "null") << std::endl;
        std::cout << "}" << std::endl;
    
    }
    
    SelectStatement() = default;
    ~SelectStatement() override;
};

class InsertStatement : public Node {
public:
    

    std::vector<std::string> columns;
    std::string table;
    
    void exec() override {

    }
};

class DeleteStatement : public Node {
public:

    std::vector<std::string> columns;
    std::string table;
    
    void exec() override {

    }
};

class CreateStatement : public Node {
public:

    std::vector<std::string> columns;
    std::string table;

    void exec() override {

    }
};

