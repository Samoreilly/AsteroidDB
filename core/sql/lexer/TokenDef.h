//to avoid duplicate headers files
#pragma once

#include <iostream>

/*
    TokenType is used to identify the type of string 'sql' is. IDENTIFIER, KEYWORD, NUMBER etc
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
    STRING,
    BOOLEAN
};

struct Token {
    TokenType token;
    std::string sql;
};

