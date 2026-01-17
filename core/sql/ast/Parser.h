#pragma once

#include "Node.h"
#include "../lexer/TokenDef.h"
#include <vector>
#include <memory>
#include "Expression.h"

class Parser {

    std::vector<Token> tokens;
    size_t pos = 0;

    //entry point
    std::unique_ptr<Node> parseStatement();
    
    std::unique_ptr<Node> parseSelect();
    std::unique_ptr<Node> parseCreate();
    std::unique_ptr<Node> parseUpdate();
    std::unique_ptr<Node> parseDelete();
    
    std::unique_ptr<Expression> parseExpression();
    std::unique_ptr<Expression> parseOrExpression();
    std::unique_ptr<Expression> parseAndExpression();
    std::unique_ptr<Expression> parseComparison();
    std::unique_ptr<Expression> parsePrimary(); 


public:

    Parser(const std::vector<Token> tokens) : tokens(std::move(tokens)) {}; 

    std::vector<Token> getTokens() const {
        return tokens;
    }

    std::unique_ptr<Node> parse() {
        return parseStatement();
    }

    const Token& peek() const {
        return tokens.at(pos);
    }

    Token next() {
        if(!isAtEnd()) {
            pos++;
        }
        return tokens.at(pos - 1);
    }

    bool isAtEnd() const {
        return pos >= tokens.size();
    }

    bool check(TokenType type, const std::string& value = "") const {
            if (isAtEnd()) return false;
            if (peek().token != type) return false;
            if (!value.empty() && peek().sql != value) return false;
            return true;
    }

    bool match(TokenType type, const std::string& value = "") {
        if (check(type, value)) {
            next();
            return true;
        }
        return false;
    }

    // this method is used to parse the next expected token
    // if token does not match thrown error as the SQL statement is not valid
    // example current tokens found = SELECT *
    // next token should be FROM
    
    Token consume(TokenType type, const std::string& expectedValue = "") {
        if (!check(type, expectedValue)) {
            std::string msg = "Expected ";
            if (!expectedValue.empty()) {
                msg += "'" + expectedValue + "'";
            } else {
                msg += "token of correct type";
            }
            msg += " but got '" + peek().sql + "'";
            throw std::runtime_error(msg);
        }
        return next();
    }
};


