#pragma once

#include <memory>
#include <vector>
#include "../ast/Expression.h"
#include "../ast/Node.h"

class Parser;

class Select {
    Parser& parser;

    bool primaryKey = false;
    bool autoIncrement = false;
    bool clustered = false;

    std::unique_ptr<Expression> parseOrExpression();
    std::unique_ptr<Expression> parseAndExpression();
    std::unique_ptr<Expression> parseComparison();
    std::unique_ptr<Expression> parsePrimary();
    std::unique_ptr<Expression> parseIn();

public:

    Select(Parser& p) : parser(p) {}
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Node> parseSelect();

};
