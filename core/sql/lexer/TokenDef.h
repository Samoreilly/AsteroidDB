//to avoid duplicate headers files

#pragma once

#include <iostream>

/*
    TokenType refers to type of the concept the 'sql' substring is
    Token is the object used in the lexer for storing the TokenType and sql statement

*/

enum Statement {
    SELECT,
    INSERT,
    CREATE,
    DELETE,
    UPDATE
};

enum TokenType { 
    IDENTIFIER, 
    KEYWORD, 
    NUMBER, 
    OPERATOR,
    COMMA,
    SYMBOL,
    STRING
};

struct Token {
    TokenType token;
    std::string sql;
};

