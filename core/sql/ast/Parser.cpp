#include "Parser.h"
#include "../lexer/TokenDef.h"
#include "../lexer/lexer.h"
#include "Node.h"

/*
This file is responsible for building the Abstract Syntax Tree,
It used Recursive Descent Parsing, where function are called based on the current to token to parse the input
This is also where errors are handled
*/


std::unique_ptr<Node> Parser::parseStatement() {

    Token p = peek();

    //cant check direct token type because token type is generic like KEYWORD, OPERATOR etc 

    Statement tokenStatement;

    if(Lexer::stringToParseStatement.find(p.sql) !=  Lexer::stringToParseStatement.end()) {
        tokenStatement = Lexer::stringToParseStatement.at(p.sql);
    }

    switch (tokenStatement) {
        
        case Statement::SELECT:
            return parseSelect();
        
        case Statement::CREATE:
            return parseCreate();
        
        case Statement::UPDATE:
            return parseUpdate();

        case Statement::DELETE:
            return parseDelete();
        
        default: throw std::runtime_error("NO PARSE TYPE COULD BE FOUND");
    };
}

std::unique_ptr<Node> Parser::parseSelect() {

    std::unique_ptr<SelectStatement> select = std::make_unique<SelectStatement>();

    consume(KEYWORD, "select");
    
    //select *
    if(check(OPERATOR, "*")) {
        next();
        select-> columns.push_back("*");
    
    }else {
        //select specific columns

        do {
            Token col = consume(IDENTIFIER);
            select->columns.push_back(col.sql);
        
        }while(match(SYMBOL, "," ));
    }
 
    consume(KEYWORD, "from");

    Token tableName = consume(IDENTIFIER);
    select-> table = tableName.sql;
    
    //check for tokens after table for WHERE clause etc;

    if(match(KEYWORD, "where")) {
        select->whereClause = parseExpression();
    }
    
    match(SYMBOL, "where");
    
    return select;
}

std::unique_ptr<Expression> Parser::parseExpression() {
    return parseOrExpression();
}

std::unique_ptr<Expression> Parser::parseOrExpression() {
    auto left = parseAndExpression();
    
    while(match(OPERATOR, "or")) {
        auto right = parseAndExpression();
        
        // Use std::make_unique instead of new
        left = std::make_unique<BinaryExpression>(
            std::move(left), 
            "or", 
            std::move(right)
        );
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseAndExpression() {
    auto left = parseComparison();
    
    while(match(OPERATOR, "and")) {
        auto right = parseComparison();
        
        left = std::make_unique<BinaryExpression>(
            std::move(left), 
            "and", 
            std::move(right)
        );
    }
    
    return left;
}

std::unique_ptr<Expression> Parser::parseComparison() {

    auto left = parsePrimary();

    if(check(OPERATOR)) {
        const std::string op = peek().sql;
        
        if(op == "==" || op == ">=" || op == "<="
            || op == "<" || op == ">" || op == "&" || op == "!="
            || op == "<>" || op == "="
        ) {
            next();
            auto right = parsePrimary();
        
            return std::make_unique<BinaryExpression>(
                std::move(left), op, std::move(right)
            );

        }
    
    }

    return left;
}

std::unique_ptr<Expression> Parser::parsePrimary() {
    
    if(match(SYMBOL, "(")) {
    //begin recursive loop again from parseExpression to build the tree
        auto expr = parseExpression();
    
        consume(SYMBOL, ")");
        return expr;
    }

    if(check(NUMBER)) {
        Token num = next();
        
        //if decimal point found, convert to double
        if(num.sql.find('.') != std::string::npos) {
            return std::make_unique<Literal>(Value(std::stod(num.sql)));
        }else{
            return std::make_unique<Literal>(Value(std::stoi(num.sql)));
        }
    }
    
    if(check(STRING)) {
        Token str = next();
        return std::make_unique<Literal>(Value(str.sql));
    }

    if(check(BOOLEAN))  {
        Token b = next();
        bool val = (b.sql == "true");
        return std::make_unique<Literal>(Value(val));
    }

    if(check(IDENTIFIER)) {
        Token id = next();
        return std::make_unique<Identifier>(id.sql);
    }
    
    if (match(OPERATOR, "-")) {
        Token num = consume(NUMBER);
        
        // if decimal point found, convert to double
        if (num.sql.find('.') != std::string::npos) {
            return std::make_unique<Literal>(Value(-std::stod(num.sql)));
        } else {
            return std::make_unique<Literal>(Value(-std::stoi(num.sql)));
        }
    }
    
    throw std::runtime_error("Expected expression, got: " + peek().sql);
}

std::unique_ptr<Node> Parser::parseCreate() {
    throw std::runtime_error("CREATE not yet implemented");
}

std::unique_ptr<Node> Parser::parseUpdate() {
    throw std::runtime_error("UPDATE not yet implemented");
}

std::unique_ptr<Node> Parser::parseDelete() {
    throw std::runtime_error("DELETE not yet implemented");
}
