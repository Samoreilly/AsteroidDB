#include "Node.h"
#include "../lexer/TokenDef.h"
#include <vector>
#include <memory>

class Parser {

    std::vector<Token> tokens;
    int pos = 0;

    //entry point
    std::unique_ptr<Node> parseStatement();
    
    std::unique_ptr<Node> parseSelect();
    std::unique_ptr<Node> parseCreate();
    std::unique_ptr<Node> parseUpdate();
    std::unique_ptr<Node> parseDelete();
    //std::unique_ptr<Node> parseSelect();
            
public:

    Parser(const std::vector<Token> tokens) : tokens(std::move(tokens)) {}; 

    std::unique_ptr<Node> parse() {
        return parseStatement();
    }

    const Token& peek() const {
        return tokens.at(pos);
    }

    Token next() {
        if(!isAtEnd()) {
            pos++;
        }
        return tokens.at(pos - 1);
    }

    bool isAtEnd() const {
        return pos >= tokens.size();
    }

    Token consume(TokenType type, std::string token) {
        if(peek().token == type) return next();
        
        throw std::runtime_error(token);
    }

};
