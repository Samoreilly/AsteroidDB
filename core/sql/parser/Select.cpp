#include <memory>
#include <vector>
#include "../ast/Expression.h"
#include "Select.h"
#include "../ast/Parser.h"
#include "../ast/Node.h"

std::unique_ptr<Node> Select::parseSelect() {

    std::unique_ptr<SelectStatement> select = std::make_unique<SelectStatement>();

    parser.consume(KEYWORD, "select");
    
    //select *
    if(parser.check(OPERATOR, "*")) {
        parser.next();
        select->columns.push_back("*");
    
    }else {
        //select specific columns

        do {
        
            Token col = parser.consume(IDENTIFIER);
            select->columns.push_back(col.sql);
        
        }while(parser.match(SYMBOL, "," ));
    }
 
    parser.consume(KEYWORD, "from");

    Token tableName = parser.consume(IDENTIFIER);
    select->table = tableName.sql;
    
    //check for tokens after table for WHERE clause etc;

    if(parser.match(KEYWORD, "where")) {
        select->whereClause = parseExpression();
    }
    
   
    return select;
}

std::unique_ptr<Expression> Select::parseExpression() {
    return parseOrExpression();
}

std::unique_ptr<Expression> Select::parseOrExpression() {
    auto left = parseAndExpression();
    
    while(parser.match(OPERATOR, "or")) {
        auto right = parseAndExpression();
        
        left = std::make_unique<BinaryExpression>(
            std::move(left), 
            "or", 
            std::move(right)
        );
    }
    
    return left;
}
    // Check for IN keyword

std::unique_ptr<Expression> Select::parseAndExpression() {
    auto left = parseComparison();
  
    while(parser.match(OPERATOR, "and")) {
        auto right = parseComparison();
        
        left = std::make_unique<BinaryExpression>(
            std::move(left), 
            "and", 
            std::move(right)
        );
    }
    
    return left;
}

std::unique_ptr<Expression> Select::parseComparison() {

    auto left = parsePrimary();

    if(parser.check(OPERATOR, "not") || parser.check(KEYWORD, "not")) {
        if(parser.peek(1).sql == "in") {
            parser.next();
            parser.consume(KEYWORD, "in");

            auto in = std::make_unique<InExpression>();
            in->left = std::move(left);
            in->isNotIn = true;

            do {

                in->values.push_back(parsePrimary());
            }while(parser.match(SYMBOL, ","));
            
            parser.consume(SYMBOL, ",");

            return in;
        } 
    }

    if(parser.check(KEYWORD, "in") || parser.check(OPERATOR, "in")) {
        parser.next();
        
        auto inExpr = std::make_unique<InExpression>();
        inExpr->left = std::move(left);
        
        parser.consume(SYMBOL, "(");
        do {
            inExpr->values.push_back(parsePrimary());
        } while(parser.match(SYMBOL, ","));
        parser.consume(SYMBOL, ")");
        
        return inExpr;
    }

    if(parser.check(OPERATOR)) {
        const std::string op = parser.peek().sql;
        
        if(op == "==" || op == ">=" || op == "<="
            || op == "<" || op == ">" || op == "&" || op == "!="
            || op == "<>" || op == "="
        ) {
            parser.next();
            auto right = parsePrimary();
        
            return std::make_unique<BinaryExpression>(
                std::move(left), op, std::move(right)
            );

        }
    
    }else if(parser.peek().sql == "in") {
        std::cout << "IN FOUND";
    }

    return left;
}

std::unique_ptr<Expression> Select::parsePrimary() {

    if(parser.match(SYMBOL, "(")) {
    //begin recursive loop again from parseExpression to build the tree
        auto expr = parseExpression();
    
        parser.consume(SYMBOL, ")");
        return expr;
    }

    if(parser.check(NUMBER)) {
        Token num = parser.next();
        
        //if decimal point found, convert to double
        if(num.sql.find('.') != std::string::npos) {
            return std::make_unique<Literal>(Value(std::stod(num.sql)));
        }else{
            return std::make_unique<Literal>(Value(std::stoi(num.sql)));
        }
    }
    
    if(parser.check(STRING)) {
        Token str = parser.next();
        return std::make_unique<Literal>(Value(str.sql));
    }

    if(parser.check(BOOLEAN))  {
        Token b = parser.next();
        bool val = (b.sql == "true");
        return std::make_unique<Literal>(Value(val));
    }

    if(parser.check(IDENTIFIER)) {
        Token id = parser.next();
        return std::make_unique<Identifier>(id.sql);
    }
    
    if (parser.match(OPERATOR, "-")) {
        Token num = parser.consume(NUMBER);
        
        // if decimal point found, convert to double
        if (num.sql.find('.') != std::string::npos) {
            return std::make_unique<Literal>(Value(-std::stod(num.sql)));
        } else {
            return std::make_unique<Literal>(Value(-std::stoi(num.sql)));
        }
    }
    
    throw std::runtime_error("Expected expression, got: " + parser.peek().sql);
}
