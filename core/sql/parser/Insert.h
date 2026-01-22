#pragma once

#include "../ast/Parser.h"

class Parser;

class Insert {

    Parser& parser;

    void parseColumns(std::unique_ptr<InsertStatement>& insertStatement, std::vector<std::string>& v);
    void parseInputs(std::unique_ptr<InsertStatement>& insertStatement);
    
    void verifyInsert(const std::unique_ptr<InsertStatement>& insertStatement, const int& size);

public:

    Insert(Parser& p) : parser(p) {}

    std::unique_ptr<Node> parseInsert();

};
