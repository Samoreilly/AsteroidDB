#include <iostream>
#include <unordered_map>
#include <string>

class Lexer {
public:

    const std::unordered_map<std::string, std::string> tokens = {
        {"select", "select"},
        {"delete", "delete"}
    };
    
    void lexer(std::string s);
    void whiteSpace();
    
}
