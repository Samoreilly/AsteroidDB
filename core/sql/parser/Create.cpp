#include "Create.h"
#include "../ast/Parser.h"
#include "../ast/Node.h"
#include <stdexcept>

std::unique_ptr<Node> Create::parseCreate() {
    
    std::unique_ptr<CreateStatement> createStatement = std::make_unique<CreateStatement>();
    
    parser.consume(KEYWORD, "create");

    std::string path = parser.consume(KEYWORD).sql;
    std::string name = parser.consume(IDENTIFIER).sql;

    //get next token to see if were creating a TABLE, database etc
    if(path == "database") {
        parser.consume(SYMBOL, "(");
        
        createStatement->table = name;
        createStatement->database = name;
            
        return createStatement;
    
    
    }else if(path == "table") {
        parser.consume(SYMBOL, "(");
        
        createStatement->table = name;
                
        do {

            CreateColumn col;
            col.name = parser.consume(IDENTIFIER).sql;
            col.type = parser.consume(IDENTIFIER).sql;
            
            createStatement->columns.push_back(col);

        }while(parser.match(SYMBOL, ","));

        parser.consume(SYMBOL, ")");
        parser.match(SYMBOL, ";");
        
        return createStatement;
        
    }

    throw std::runtime_error("PLEASE SPECIFY CREATING A TABLE OR DATABASE");
    
    return createStatement;

}

