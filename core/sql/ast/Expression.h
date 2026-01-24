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
        (void)indent; // unused for now
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
        std::string i(indent, ' ');
        std::cout << i << "BinaryExpression(" << op << ") {" << std::endl;

        std::cout << i << "  left:" << std::endl;
        left->print(indent + 4);

        std::cout << i << "  right:" << std::endl;
        right->print(indent + 4);

        std::cout << i << "}" << std::endl;
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
      
    std::vector<std::unique_ptr<Expression>> method;
    std::string methodName;
    

    explicit MethodExpression(std::vector<std::unique_ptr<Expression>> args)
        : method(std::move(args)) {}
    
    Value eval(Executor* executor) override {
        (void)executor; // unused
        throw std::runtime_error("MethodExpression not implemented");
    }


    void print(int indent = 0) const override {
        std::string i(indent, ' ');
        std::cout << i << "MethodExpression(" << methodName << ") {" << std::endl;

        std::cout << i << "  arguments: [" << std::endl;
        for (const auto& arg : method) {
            arg->print(indent + 4);
        }
        std::cout << i << "  ]" << std::endl;

        std::cout << i << "}" << std::endl;
    }

};

class Literal : public Expression {
public:
    Value value;
    
    Literal(Value v) : value(std::move(v)) {}
    
    Value eval(Executor* executor) override {
        (void)executor; // unused
        return value;
    }
    
    void print(int indent = 0) const override {
        std::string i(indent, ' ');
        std::cout << i << "Literal(" << value << ")" << std::endl;
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
        std::string i(indent, ' ');
        std::cout << i << "Identifier(" << token << ")" << std::endl;
    }


};
