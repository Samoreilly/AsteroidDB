#include <iostream>
#include "../lexer/TokenDef.h"

class Expression {

};

class BinaryExpression : Expression {

    //cases like id > 2, etc
    Expression* left;
    std::string op;
    Expression* right;

    BinaryExpression(Expression* left, std::string op, Expression* right)
        : left(left), op(op), right(right) {}

    ~BinaryExpression() {
        delete left;
        delete right;
    }
};

class Literal : Expression {
public:

    TokenType token;
    std::string value;

};

class Identifier : Expression {
public:

    std::string token;


};
