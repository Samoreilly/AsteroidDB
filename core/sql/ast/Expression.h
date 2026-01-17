#pragma once

#include <iostream>
#include "../lexer/TokenDef.h"
#include "Node.h"
#include "Value.h"
#include "../../engine/Executor.h"

class Executor;

class Expression : public Node {
public:

    //value is char, int, bool. string, null
    virtual ~Expression();
    virtual Value eval(Executor* executor) = 0;

};

class BinaryExpression : public Expression {
public:
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
    
    Value eval(Executor* executor) override {
        Value leftVal = left->eval(executor);   
        Value rightVal = right->eval(executor); 
        
    if (op == ">")  return Value(leftVal > rightVal);   // Uses Value::operator>
    if (op == "<")  return Value(leftVal < rightVal);   // Uses Value::operator
    if (op == "=")  return Value(leftVal == rightVal);  // Uses Value::operator==
    if (op == "!=") return Value(leftVal != rightVal);  // Uses Value::operator!=
    if (op == ">=") return Value(leftVal >= rightVal);  // Uses Value::operator>=
    if (op == "<=") return Value(leftVal <= rightVal);  // Uses Value::operator<=
    if (op == "and") return Value(leftVal.asBool() && rightVal.asBool());
    if (op == "or")  return Value(leftVal.asBool() || rightVal.asBool());    }
};


class Literal : public Expression {
public:

    Value value;
    Literal(Value v) : value(v) {};

    Value eval(Executor* executor) override {
        return value;
    }
    
};

class Identifier : public Expression {
public:

    std::string token;

    Value eval(Executor* executor) override {
        return executor->getCurrentRow().at(token);
    }
};

