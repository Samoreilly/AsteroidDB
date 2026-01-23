#include <iostream>
#include <string>
#include <vector>
#include "sql/lexer/lexer.h"
#include "sql/ast/Parser.h"
#include "engine/executor/ExecutorEngine.h"

int main() {
    std::cout << "AsteroidDB Interactive Shell" << std::endl;
    std::cout << "Type 'exit' to quit." << std::endl;
    
    executor::ExecutorEngine engine(".");
    std::string input;
    
    while (true) {
        std::cout << "AsteroidDB> ";
        std::string line;
        if (!std::getline(std::cin, line)) break;
        
        if (line == "exit" || line == "quit") break;
        if (line.empty()) continue;
        
        try {
            Lexer lexer;
            lexer.lexer(line);
            
            if (lexer.getTokens().empty()) continue;
            
            Parser parser(lexer.getTokens());
            std::unique_ptr<Node> ast = parser.parse();
            
            if (ast) {
                engine.execute(ast.get());
            } else {
                std::cout << "Error: Failed to parse statement." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    
    std::cout << "Goodbye!" << std::endl;
    return 0;
}
