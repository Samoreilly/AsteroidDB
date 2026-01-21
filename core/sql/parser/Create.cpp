#include "../ast/Expression.h"
#include "Select.h"
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
            
            Token tok = parser.consume(IDENTIFIER);

            std::string dataType = tok.sql;

            if(parser.check(SYMBOL, "(")) {
                col.type = parseVariableLength(tok); 
            }else {
               col.type = dataType; 
            }
           
            //if there is constraints
            std::string checkNext = parser.peek().sql;
        
            //CREATE TABLE Products (ProductID INT PRIMARY KEY, ProductName INTEGER, Price DECIMAL, InStock BOOLEAN);
            //TODO: Clustered must be paired
            
            while(parser.check(KEYWORD) || parser.check(OPERATOR) || Parser::CONSTRAINT_KEYWORDS.find(checkNext) != Parser::CONSTRAINT_KEYWORDS.end()) {

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
                    parser.consume(IDENTIFIER, "foreign");
                    parser.consume(IDENTIFIER, "key");
                    
                    col.constraints.push_back("foreign key");

                    if(parser.check(IDENTIFIER, "references")) {
                        parser.consume(IDENTIFIER, "references");
                        parseForeignKey(createStatement);
                    }
                
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
                    parser.consume(OPERATOR, "not");
                    parser.consume(IDENTIFIER, "null");
           
                    col.constraints.push_back("not null");
                
                }else if(con == "auto_increment") {
                    parser.consume(KEYWORD, "auto_increment");
                    col.constraints.push_back("auto_increment");
                    
                }else if(con == "check") {
                    //quantity INT CHECK (quantity > 0)
                    parser.consume(IDENTIFIER, "check");
                    parser.consume(SYMBOL, "(");
                    
                    std::string checkName = parser.peek().sql;
                    
                    if(col.name != checkName) {
                        throw std::runtime_error("COLUMN NAME MUST MATCH NAME INSIDE OF CHECK STATEMENT");
                    }
                    
                    Select selectParser(parser);
                    auto check = selectParser.parseExpression(); 

                    parser.consume(SYMBOL, ")");

                    col.check = std::make_unique<CheckExpression>(std::move(check));

                    
                }else {
                    std::cerr << "UNKOWN CONSTRAINT USED";
                    parser.consume(IDENTIFIER); 
                }


            }

            createStatement->columns.push_back(std::move(col));
    
        // check if there's a comma followed by another column or a constraint
        if(parser.match(SYMBOL, ",")) {
            
            if(parser.check(IDENTIFIER, "foreign") || parser.check(IDENTIFIER, "unique") || 
               parser.check(IDENTIFIER, "primary") || parser.check(KEYWORD, "foreign") || 
               parser.check(KEYWORD, "unique") || parser.check(KEYWORD, "primary")) {
                // break out of column parsing to parse constraints
                break;
            }

            // continue parsing
        } else {
            // no commas, then exit and parse table level constraints like foreign keys
            break;
        }        
        }while(true);

        //above parses columns
        //parse table level constraints

        while(parser.check(IDENTIFIER)) {
            std::string constraint = parser.peek().sql;
            
            if(constraint == "foreign") {
                parser.consume(IDENTIFIER, "foreign");
                parser.consume(IDENTIFIER, "key");
                parseForeignKey(createStatement);
                
                parser.match(SYMBOL, ",");
            } else {
                break;
            }
        }
        parser.consume(SYMBOL, ")");
        parser.match(SYMBOL, ";");
        
        return createStatement;
        
    }

    throw std::runtime_error("PLEASE SPECIFY CREATING A TABLE OR DATABASE");
    
    return createStatement;

}

std::string Create::parseVariableLength(const Token& dataType) {

    std::string builder;
    builder.reserve(15);
    
    parser.consume(SYMBOL, "(");

    if(parser.SQL_TYPES.find(dataType.sql) == parser.SQL_TYPES.end()) {
        std::cout << dataType.sql;
        throw std::runtime_error("DATA TYPE DOESN'T EXIST");
    }

    builder += dataType.sql;
    builder += "(";

    while(!parser.check(SYMBOL, ")")) {

        builder += parser.consume(NUMBER).sql;

        if(parser.match(SYMBOL, ",")) {
            builder += ",";
        }
    }

    builder += parser.consume(SYMBOL, ")").sql;

    return builder;
}

void Create::parseForeignKey(const std::unique_ptr<CreateStatement>& createStatement, const std::string& columnName) {
    
    ForeignKey fk;
    
    //this handles column lever foreign keys and table level;
    if(!columnName.empty()) {
        //e.g. // Column-level: columnName already known
        fk.columnNames.push_back(columnName);
    } else {
        
        //FOREIGN KEY (CustomerID) REFERENCES Customers(CustomerID)
        parser.consume(SYMBOL, "(");
        do {
            fk.columnNames.push_back(parser.consume(IDENTIFIER).sql);
        
        }while(parser.match(SYMBOL, ","));
        
        parser.consume(SYMBOL, ")");
        parser.consume(IDENTIFIER, "references");
    }

    fk.referencedTable = parser.consume(IDENTIFIER).sql;
    parser.consume(SYMBOL, "(");

    do {
        fk.referencedColumns.push_back(parser.consume(IDENTIFIER).sql);
    }while(parser.match(SYMBOL, ","));

    parser.consume(SYMBOL, ")");

    createStatement->foreignKeys.push_back(fk);
}






