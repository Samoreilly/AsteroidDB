#pragma once

#include <memory>
#include <vector>
#include "../ast/Expression.h"
#include "../ast/Node.h"

class Parser;

class Select {
    Parser& parser;

    std::unique_ptr<Expression> parseOrExpression();
    std::unique_ptr<Expression> parseAndExpression();
    std::unique_ptr<Expression> parseComparison();
    std::unique_ptr<Expression> parsePrimary();

public:
    Select(Parser& p) : parser(p) {}
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Node> parseSelect();

};
