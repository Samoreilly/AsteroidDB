#include "Parser.h"
#include "../lexer/TokenDef.h"
#include "../lexer/lexer.h"
#include "Node.h"

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

    return select;
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
