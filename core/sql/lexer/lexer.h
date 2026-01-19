#pragma once

#include <cctype>
#include <iostream>
#include <unordered_map>
#include <string>
#include <vector>
#include "TokenDef.h"

class Lexer {
public:

    static inline const std::unordered_map<std::string, TokenType> KEYWORDS = {
        {"select", TokenType::KEYWORD},
        {"delete", TokenType::KEYWORD},
        {"create", TokenType::KEYWORD},
        {"insert", TokenType::KEYWORD},
        {"update", TokenType::KEYWORD},
        {"drop",   TokenType::KEYWORD},
        {"from",   TokenType::KEYWORD},
        {"where",  TokenType::KEYWORD},
        {"into",   TokenType::KEYWORD},
        {"values", TokenType::KEYWORD},
        {"table",  TokenType::KEYWORD},
        {"set", TokenType::KEYWORD}
    };


    static inline const std::unordered_map<std::string, TokenType> OPERATORS = {

        {"+",  TokenType::OPERATOR},
        {"-",  TokenType::OPERATOR},
        {"*",  TokenType::OPERATOR},
        {"/",  TokenType::OPERATOR},
        {"%",  TokenType::OPERATOR},

        {"!", TokenType::OPERATOR},
        {"=",  TokenType::OPERATOR},
        {"!=", TokenType::OPERATOR},
        {"<>", TokenType::OPERATOR},
        {"<",  TokenType::OPERATOR},
        {">",  TokenType::OPERATOR},
        {"<=", TokenType::OPERATOR},
        {">=", TokenType::OPERATOR},

        {"and", TokenType::OPERATOR},
        {"or",  TokenType::OPERATOR},
        {"not", TokenType::OPERATOR},

    
        {"in",     TokenType::OPERATOR},
        {"like",   TokenType::OPERATOR},
        {"between",TokenType::OPERATOR},
        {"is",     TokenType::OPERATOR},
        {"is null",     TokenType::OPERATOR},
        {"is not null", TokenType::OPERATOR}
    };
    
    static inline const std::unordered_map<std::string, TokenType> SYMBOLS = {
        {",", TokenType::SYMBOL},
        {";", TokenType::SYMBOL},
        {"(", TokenType::SYMBOL},
        {")", TokenType::SYMBOL},
        {".", TokenType::SYMBOL},
        {"'", TokenType::SYMBOL}

    };

    static inline const std::unordered_map<std::string, Statement> stringToParseStatement = {
        {"select", Statement::SELECT},
        {"insert", Statement::INSERT},
        {"delete", Statement::DELETE},
        {"update", Statement::UPDATE},
        {"create", Statement::CREATE}
    };


    bool keywords_contains(const std::string& str) {
        return KEYWORDS.find(str) != KEYWORDS.end();
    }

    std::vector<Token> tokens;
    
    int startIndex = 0, endIndex = 0, LENGTH = 0;
    bool stop = false;

    std::vector<Token> lexer(std::string str);
    bool addToken(const std::string& token);
    void createToken(std::string_view token);
    
    std::vector<Token> getTokens() const {
        return tokens;
    }

private:
    
    void consec_symbols(std::string& token);

    void printTokens() {
        
        for(Token t : tokens) {
        
            std::cout << "TokenType: " << tokenTypeToString(t.token) << "\n" << "SQL token:" << t.sql << "\n";
        }
    }

    std::string tokenTypeToString(TokenType t) {
        
        switch(t) {
            case IDENTIFIER: return "IDENTIFIER";
            case KEYWORD:    return "KEYWORD";
            case NUMBER:     return "NUMBER";
            case OPERATOR:   return "OPERATOR";
            case COMMA:      return "COMMA";
            case SYMBOL:     return "SYMBOL";
            case STRING:     return "STRING";
            case BOOLEAN:    return "BOOLEAN";
            default:         return "UNKNOWN";
        }
    }
    
    bool isOperator(const std::string str) {
        return OPERATORS.find(str) != OPERATORS.end();
    }

    bool isSymbolOrWhiteSpace(char c) {
        if(std::isspace(c)) {
            return true;
        }else if (isSymbol(c)) {
            return true;
        }
        return false;
    }

    bool whiteSpace(char c) {
        if (std::isspace(c)) {
            return true;
        }
        return false;
    }

    bool isSymbol(char c) {
        if (c == '(' || c == ')' || c == ',' || c == ';' || c == '\'') {
            return true;
        }
        return false;
    }
    
};
