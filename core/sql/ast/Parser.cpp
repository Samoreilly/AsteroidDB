#include "Parser.h"
#include "../lexer/TokenDef.h"
#include "../lexer/lexer.h"
#include "Node.h"
#include "../parser/Create.h"
#include "../parser/Select.h"
#include "../parser/Insert.h"
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
        
        case Statement::SELECT: {
            Select selectParser(*this);
            return selectParser.parseSelect();
        }
        
        case Statement::CREATE: {
            Create createParser(*this);
            return createParser.parseCreate();
        }

        case Statement::INSERT: {
            Insert parseInsert(*this);
            return parseInsert.parseInsert();
        }
        
        case Statement::UPDATE:
            throw std::runtime_error("UPDATE not yet implemented");

        case Statement::DELETE:
            throw std::runtime_error("DELETE not yet implemented");
        
        default: throw std::runtime_error("NO PARSE TYPE COULD BE FOUND");
    };
}


