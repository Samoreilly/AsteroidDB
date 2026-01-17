#pragma once
#include <iostream>
#include <memory>  // Add this for std::unique_ptr
#include "../lexer/TokenDef.h"
#include "Node.h"
#include "Value.h"
#include "../../engine/Executor.h"


class Executor;


class Expression{
public:
    virtual ~Expression() = default;
    virtual Value eval(Executor* executor) = 0;
    virtual void print(int indent = 0) const = 0;
};

class BinaryExpression : public Expression {
public:

    std::unique_ptr<Expression> left;
    std::string op;
    std::unique_ptr<Expression> right;

    BinaryExpression(std::unique_ptr<Expression> l, std::string o, std::unique_ptr<Expression> r)
        : left(std::move(l)), op(std::move(o)), right(std::move(r)) {}
    
    ~BinaryExpression() override = default;
    
    Value eval(Executor* executor) override {
        Value leftVal = left->eval(executor);   
        Value rightVal = right->eval(executor); 
        
        if (op == ">")  return Value(leftVal > rightVal);
        if (op == "<")  return Value(leftVal < rightVal);
        if (op == "=")  return Value(leftVal == rightVal);
        if (op == "!=") return Value(leftVal != rightVal);
        if (op == ">=") return Value(leftVal >= rightVal);
        if (op == "<=") return Value(leftVal <= rightVal);
        if (op == "and") return Value(leftVal.asBool() && rightVal.asBool());
        if (op == "or")  return Value(leftVal.asBool() || rightVal.asBool());
        
        throw std::runtime_error("Unknown operator: " + op);
    }
    
    void print(int indent = 0) const override {
        std::string indentStr(indent, ' ');
        std::cout << indentStr << "BinaryExpression(" << op << ") {" << std::endl;
        std::cout << indentStr << "  left: ";
        left->print(indent + 4);
        std::cout << indentStr << "  right: ";
        right->print(indent + 4);
        std::cout << indentStr << "}" << std::endl;
    }
};

class Literal : public Expression {
public:
    Value value;
    
    Literal(Value v) : value(std::move(v)) {}
    
    Value eval(Executor* executor) override {
        return value;
    }
    
    void print(int indent = 0) const override {
        std::cout << "Literal(" << value << ")" << std::endl;
    }
};

class Identifier : public Expression {
public:
    std::string token;
    
    Identifier(std::string t) : token(std::move(t)) {}
    
    Value eval(Executor* executor) override {
        return executor->getCurrentRow().at(token);
    }

    void print(int indent = 0) const override {
        std::cout << "Identifier(" << token << ")" << std::endl;
    }

};
