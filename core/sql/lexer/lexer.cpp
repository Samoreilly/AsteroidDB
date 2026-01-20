#include "lexer.h"
#include "TokenDef.h"
#include <boost/algorithm/string/case_conv.hpp>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <string_view>
#include <vector> 


std::vector<Token> Lexer::lexer(std::string str) { 

    //std::cout << "ENTERING LEXER" << "\n";

    LENGTH = str.length();
    
    boost::algorithm::to_lower(str);

    
    while(endIndex < LENGTH) {
        char c = str[endIndex];
     
        if(whiteSpace(c)) {
                    
            while(endIndex < LENGTH && whiteSpace(str[endIndex])) endIndex++;
            
            startIndex = endIndex;
            continue;
        } 

        if(isOperator(std::string(1, c))) {
          
            startIndex = endIndex;
            endIndex++;
            std::string_view tokenView(str.data() + startIndex, endIndex - startIndex);
            
            // Continue scanning for multi-character operators like >=
            while (endIndex < LENGTH) {
                std::string_view potentialOp = std::string_view(str.data() + startIndex, endIndex + 1 - startIndex);

                if(OPERATORS.find(std::string(potentialOp)) != OPERATORS.end()) {
                    tokenView = potentialOp;
                    endIndex++;
                } else {
                    break;
                }
            }
            
            createToken(tokenView);
            startIndex = endIndex;
            continue;
        }
            
        if(isSymbol(c)) {

            if(c == '\'') {
                
                // hanlding string literals e.g. 'sam'
                // this lexer is designed to exclude the string literals and just include the value inside
                // so it would be sam for the above case


                startIndex = ++endIndex;
                
                while(endIndex < LENGTH && str[endIndex] != '\'') {
                    endIndex++;
                }

                if(endIndex >= LENGTH) {
                    std::cout << "No terminating string literal";
                    throw std::runtime_error("No terminating string literal");
                }

                std::string_view stringLiteral(str.data() + startIndex, endIndex - startIndex);

                tokens.push_back({TokenType::STRING, std::string(stringLiteral)});
            
                startIndex = ++endIndex;
                continue;

            }else {
             
                std::string_view tokenView(str.data() + startIndex, 1);
                createToken(tokenView);
            }

            startIndex = ++endIndex; 
            continue;
        }

        //if not symbol or space. continue
        while(endIndex < LENGTH && 
              !whiteSpace(str[endIndex]) && 
              !isSymbol(str[endIndex]) &&
              !isOperator(std::string(1, str[endIndex]))) {
            endIndex++;
        }
        
        if(endIndex > startIndex) {

            std::string_view tokenView(str.data() + startIndex, endIndex - startIndex);
            createToken(tokenView);
            startIndex = endIndex;
        }

        
    }
    

    return tokens;
    // std::cout << "PRINT" << "\n"; 
    //printTokens();
    
}

bool Lexer::addToken(const std::string& token) {
    
    if(token.empty()) {
        return false;
    }
    
    // std::cout << "addToken" << "\n";

    if(KEYWORDS.find(token) != KEYWORDS.end()) {
        //std::cout << "added keyword: " << token << "\n\n";
        tokens.push_back({TokenType::KEYWORD, token});
        return true;
    
    }else if(std::isdigit(token[0])) {    
        tokens.push_back({TokenType::NUMBER, token});
        return true;
    
    }else if(OPERATORS.find(token) != OPERATORS.end()){
        tokens.push_back({TokenType::OPERATOR, token});
        return true;
    
    }else if(SYMBOLS.find(token) != SYMBOLS.end()){
        tokens.push_back({TokenType::SYMBOL, token});
        return true;
    }else if(token == "true" || token == "false") {
        tokens.push_back({TokenType::BOOLEAN, token});
        return true;
    }else {    
        tokens.push_back({TokenType::IDENTIFIER, token});
        return true;
    }

    return false;
} 

void Lexer::createToken(std::string_view token){
    addToken(std::string(token));
}



