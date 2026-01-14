#include "lexer.h"
#include <iostream>
#include <boost/algorithm/string.hpp>

std::vector>

void Lexer::lexer(std::string str) { 
    int index = 0, start = 0; 
    
    boost::algorithm::to_lower(str);
    
    auto whiteSpace = [this](char c) -> bool {
        if(std::isspace(c)){
            this->index++; 
            return true; nf
        }
    };

    auto isSymbol = [this](char c) -> bool {
        if(c == '*' || c == '(' || c == ')' || c == ',' || c == ';') {
            this->index++;
            return true;
        }
    };

    std::string_view chunk;
    
    while(index < str.length()) {
        char c = str[index];

        if(whiteSpace(c) || isSymbol(c))continue;
        
 
        

        if(Lexer::tokens.contains()) {

        }
I


    }

}
