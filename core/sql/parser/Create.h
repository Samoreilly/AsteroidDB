#pragma once

#include "../ast/Expression.h"
#include "../ast/Node.h"
#include <memory>

class Parser;

class Create { 
    Parser& parser;

    std::string parseVariableLength(const Token& tok);

public:

    Create(Parser& p) : parser(p) {}
    
    std::unique_ptr<Node> parseCreate();
    
    
    // Expression parsing is unused in Create based on Node.h types, but keeping declarations if needed or removing if generic. 
    // Given the refactor, if Create uses expressions, it might use Select's or its own. 
    // For now keeping structure but fixing syntax.
};
