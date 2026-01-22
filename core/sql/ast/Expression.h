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

//when executing check if value in the database is in the vector
class InExpression : public Expression {
public:

    std::unique_ptr<Expression> left;
    std::vector<std::unique_ptr<Expression>> values;
    bool isNotIn = false;
    
    Value eval(Executor* executor) {
        return left->eval(executor);
    }

    void print(int indent = 0) const override {
        std::cout << "InExpression";
        if (isNotIn) std::cout << "(NOT)";
        std::cout << " {" << std::endl;
        std::cout << "  left: ";
        left->print();
        std::cout << std::endl << "  values: [";
        for (size_t i = 0; i < values.size(); ++i) {
            values[i]->print();
            if (i < values.size() - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
        std::cout << "}" << std::endl;
    }
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

class CheckExpression : public Expression {
public:
    
    std::unique_ptr<Expression> cond;

    explicit CheckExpression(std::unique_ptr<Expression> condition)
        : cond(std::move(condition)) {}

    Value eval(Executor* executor) override {
        return cond->eval(executor);
    }

    void print(int indent = 0) const override {
        std::string indentStr(indent, ' ');
        std::cout << indentStr << "CheckExpression {" << std::endl;
        cond->print(indent + 2);
        std::cout << indentStr << "}" << std::endl;
    }
};

class MethodExpression : public Expression {
public:
      
    std::unique_ptr<Expression> method;
    std::string methodName;
    
    explicit MethodExpression(std::unique_ptr<Expression> methodUsed)
        : method(std::move(methodUsed)) {}
    
    Value eval(Executor* executor) override {
        return method->eval(executor);
    }

    void print(int indent = 0) const override { 
        std::string indentation(indent, ' ');
        std::cout << indentation << "MethodExpression(" << methodName << ") {" << std::endl;
        if (method) {
            method->print(indent + 4);
        }
        std::cout << indentation << "}" << std::endl;
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
