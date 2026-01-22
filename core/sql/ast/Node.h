#pragma once

#include <iostream>
#include <vector>
#include "../lexer/TokenDef.h"
#include "Expression.h"
#include <memory>

class CheckExpression;
class Expression;

struct CreateColumn {
    std::string name;        // Column name
    std::string type;        // INT, VARCHAR, DECIMAL, etc.
    std::vector<std::string> constraints;  // PRIMARY KEY, NOT NULL, etc.
    std::unique_ptr<CheckExpression> check;
};

struct ForeignKey {
    std::string constraintName;
    std::vector<std::string> columnNames;
    std::string referencedTable;
    std::vector<std::string> referencedColumns;
};

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
    
    void print() const;
    
    SelectStatement() = default;
    ~SelectStatement() override;
};

class InsertStatement : public Node {
public:
    
    std::string table;

    std::vector<std::string> columns;
    std::vector<std::unique_ptr<Expression>> inputs;

    void exec() override {

    }

    void print() const;

    InsertStatement() = default;
    ~InsertStatement() override;
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

    std::vector<CreateColumn> columns;
    std::vector<ForeignKey> foreignKeys;

    std::string table;
    std::string database;
    
    bool primaryKey = false;
    bool clustered = false;

    void exec() override {

    }
    
    void print() const;
    
    CreateStatement() = default;
    ~CreateStatement() override;

};


