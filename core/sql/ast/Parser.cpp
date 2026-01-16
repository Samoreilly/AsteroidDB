#include "Parser.h"
#include "../lexer/TokenDef.h"
#include "../lexer/lexer.h"

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
