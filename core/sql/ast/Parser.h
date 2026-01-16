
#include "../lexer/TokenDef.h"
#include <vector>

class Parser {
public:

    Parser(const std::vector<Token> tokens) : tokens(std::move(tokens)) {}; 


private:
    std::vector<Token> tokens;

};
