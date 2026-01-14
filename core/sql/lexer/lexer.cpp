#include "lexer.h"
#include "TokenDef.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <vector> 


void Lexer::lexer(std::string str) { 
    
    std::cout << "ENTERING LEXER" << "\n";

    const int LENGTH = str.length();
 
    
    boost::algorithm::to_lower(str);
    

    while(endIndex < LENGTH) {
        char c = str[endIndex];
        
        if(whiteSpace(c)){
            
            if(endIndex > startIndex) {
               createToken(str);
            }
            endIndex++;
            startIndex = endIndex;
            continue;
        }

        if(isSymbol(c)) {
            
            if(endIndex > startIndex) {
               createToken(str);
            }
        
            consec_symbols(str);
            
            
            endIndex++;
            startIndex = endIndex;
            continue;
        }

        /*
            This will build the string until it hits a whitespace and breaks
        */
        
        endIndex++;
    }

    if(endIndex > startIndex) {
        createToken(str);
    }

    // std::cout << "PRINT" << "\n"; 
    printTokens();
    
}

bool Lexer::addToken(const std::string_view& token) {
    
    if(token.empty()) {
        std::cout << "Token is empty" << "\n";
        return false;
    }
    
    auto it = KEYWORDS.find(token);
    
    // std::cout << "addToken" << "\n";

    if(it != KEYWORDS.end()) {
        std::cout << "added keyword: " << token << "\n\n";
        tokens.push_back({TokenType::KEYWORD, token});
        return true;
    
    }else if(std::isdigit(token[0])) {
        std::cout << "added digit: " << token << "\n\n";
        
        tokens.push_back({TokenType::NUMBER, token});
        return true;
    
    }else if(token.length() == 1) {

        if(isSymbol(token[0])) {
            std::cout << "added symbol: " << token << "\n\n";
        
            tokens.push_back({TokenType::SYMBOL, token});
            return true;
        }
       
    }else {
         tokens.push_back({TokenType::IDENTIFIER, token});
        return true;
    }

    return false;
} 

void Lexer::consec_symbols(std::string& token) {
    int symbol_index = endIndex;
        
    while(endIndex < token.length() && isSymbol(token[endIndex])) {
        endIndex++;
    }

    if(endIndex > symbol_index) {
        
        std::string_view consec_symbols(token.data() + symbol_index, endIndex - symbol_index);
        addToken(consec_symbols);
    }
    startIndex = endIndex;
}

void Lexer::createToken(std::string& token){

    if(endIndex > token.length()) {
        return;
    }

    std::string_view tokenView(token.data() + startIndex, endIndex - startIndex);

    addToken(tokenView);
}



