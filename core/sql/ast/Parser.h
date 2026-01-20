#pragma once

#include "Node.h"
#include "../lexer/TokenDef.h"
#include <stdexcept>
#include <vector>
#include <memory>
#include "Expression.h"
#include <unordered_set>
#include <optional>

class Parser {

    std::vector<Token> tokens;
    size_t pos = 0;

    //entry point
    std::unique_ptr<Node> parseStatement();
  
public:

    Parser(const std::vector<Token> tokens) : tokens(std::move(tokens)) {}; 


    static inline const std::unordered_set<std::string> CONSTRAINT_KEYWORDS = {

        "primary",      // PRIMARY KEY
        "foreign",      // FOREIGN KEY
        "unique",       // UNIQUE
        "not",          // NOT NULL
        "default",      // DEFAULT value
        "check",        // CHECK condition
        "auto_increment", // AUTO_INCREMENT (MySQL)
        "identity",     // IDENTITY (SQL Server)
        "autoincrement", // AUTOINCREMENT (SQLite)
        
        "clustered",    // CLUSTERED index
        "nonclustered", // NONCLUSTERED index
        "index",        // INDEX
        
        "references"    // REFERENCES table(column)
    };

    static inline const std::unordered_set<std::string> SQL_TYPES = {
        // numeric
        "int",
        "integer",
        "smallint",
        "bigint",
        "tinyint",
        "decimal",
        "numeric",
        "float",
        "real",
        "double",

        // string
        "char",
        "varchar",
        "nchar",
        "nvarchar",
        "text",
        "clob",

        // date/time
        "date",
        "time",
        "datetime",
        "timestamp",
        "year",

        // boolean
        "boolean",
        "bool",

        // binary
        "binary",
        "varbinary",
        "blob",

        // auto / identity
        "serial",
        "bigserial",
        "identity",
        "auto_increment",

        // special
        "uuid",
        "json",
        "jsonb",
        "enum",
        "set"
    };

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
    
    bool peekNext(const std::string& expected = "") {
        if(pos + 1 >= tokens.size())throw std::runtime_error("MALFORMED SQL");

        if(!expected.empty() && tokens.at(pos + 1).sql == expected) {
            return true;
        } 

        return false;
    }

    bool lookBack(size_t n, std::string expected = "") {
        if(pos - n < 0) return false;
            
        if(tokens.at(pos - n).sql == expected) {
            return true;
        }
        
        return false;

    }
};


