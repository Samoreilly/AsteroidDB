
#include "Create.h"
#include "../ast/Parser.h"
#include "../ast/Node.h"
#include <stdexcept>
#include <string>

std::unique_ptr<Node> Create::parseCreate() {
    
    std::unique_ptr<CreateStatement> createStatement = std::make_unique<CreateStatement>();
    
    parser.consume(KEYWORD, "create");

    std::string path = parser.consume(KEYWORD).sql;
    
    std::string name = parser.consume(IDENTIFIER).sql;

    //get next token to see if were creating a TABLE, database etc
    if(path == "database") {
        
        createStatement->database = name;
        parser.match(SYMBOL, ";");

        return createStatement;

    }else if(path == "table") {
    
        createStatement->table = name;
        parser.consume(SYMBOL, "(");
        
        do {

            CreateColumn col;
            col.name = parser.consume(IDENTIFIER).sql;            
            
            std::string dataType = parser.peek().sql;
                
            if(dataType == "varchar") {
                col.type = parseVarchar();
            
            }else if(dataType == "decimal") {
                col.type = parseDecimal();
            
            }else {
                col.type = parser.consume(IDENTIFIER).sql;
            }
            //if there is constraints

            std::string checkNext = parser.peek().sql;
        
            //CREATE TABLE Products (ProductID INT PRIMARY KEY, ProductName INTEGER, Price DECIMAL, InStock BOOLEAN);

            //TODO: Clustered must be paired with primary key

            std::cout << "CURRENT TOKEN: " << parser.peek().sql << "\n";

              
            while(parser.check(KEYWORD)) {

                std::string con = parser.peek().sql;
                
                if(Parser::CONSTRAINT_KEYWORDS.find(con) == Parser::CONSTRAINT_KEYWORDS.end()) {    
                    break;
                }

                if(con == "primary") {
                    parser.consume(KEYWORD, "primary");
                    parser.consume(IDENTIFIER, "key");
                    
                    if(createStatement->primaryKey) {
                        throw std::runtime_error("ONLY 1 PRIMARY KEY IS ALLOWED PER TABLE");
                    }

                    createStatement->primaryKey = true;
                    col.constraints.push_back("primary key");
                
                }else if(con == "foreign") {
                    parser.consume(KEYWORD, "foreign");
                    parser.consume(IDENTIFIER, "key");

                    col.constraints.push_back("foreign key");
                
                }else if(con == "unique") {
                    parser.consume(KEYWORD, "unique");
                    col.constraints.push_back("unique");
                
                }else if(con == "clustered") {
                    parser.consume(KEYWORD, "clustered");
                        
                    bool foundPair = false;
                    for(const auto& c : col.constraints) {
                        if(c == "primary key" || c == "unique") {
                            foundPair = true;
                        }
                    }

                    if(!foundPair) {
                        throw std::runtime_error("CLUSTERED MUST BE PAIRED");
                    }

                    col.constraints.push_back("clustered");
                
                }else if(con == "not") {
                    parser.consume(IDENTIFIER, "not");
                    parser.consume(IDENTIFIER, "null");

                    col.constraints.push_back("NOT NULL");
                
                }else if(con == "auto_increment") {
                    parser.consume(KEYWORD, "auto_increment");

                    col.constraints.push_back("auto_increment");
                
                }else {
                    std::cerr << "UNKOWN CONSTRAINT USED";
                    parser.consume(IDENTIFIER); 
                }


            }

            createStatement->columns.push_back(col);
        
            std::cout << "CURRENT TOKEN: " << parser.peek().sql << "\n";
        }while(parser.match(SYMBOL, ","));
      
        parser.consume(SYMBOL, ")");
        parser.match(SYMBOL, ";");
        
        return createStatement;
        
    }

    throw std::runtime_error("PLEASE SPECIFY CREATING A TABLE OR DATABASE");
    
    return createStatement;

}


std::string Create::parseVarchar() {

    parser.consume(IDENTIFIER, "varchar");
    parser.consume(SYMBOL, "(");
    std::string size = parser.consume(NUMBER).sql;
    
    parser.consume(SYMBOL, ")");
    
    std::string res = "varchar(" + size + ")";
    std::cout << res;
    return res; 
}

std::string Create::parseDecimal() {

    std::string builder;
    
    //sets initial capacity of string in characters
    builder.reserve(10);

    parser.consume(IDENTIFIER, "decimal");
    parser.consume(SYMBOL, "(");

    while(!parser.check(SYMBOL, ")")) {

         builder += parser.consume(NUMBER).sql;

        if(parser.match(SYMBOL, ",")){
            builder += ",";
        }

    }
    
    std::string res  = "decimal(" + builder + ")";
    std::cout << res;
    return res;
}

