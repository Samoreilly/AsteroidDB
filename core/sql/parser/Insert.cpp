#include "Insert.h"
#include "Select.h"
#include "../ast/Parser.h"
#include "../ast/Node.h"
#include <memory>


std::unique_ptr<Node> Insert::parseInsert() {

    std::unique_ptr<InsertStatement> insertStatement = std::make_unique<InsertStatement>();


    /*
        All these 'parser.consumes..' are expected words.
        Any that dont match should fail
    */

    parser.consume(KEYWORD, "insert");
    parser.consume(KEYWORD, "into");
    
    std::string tableName = parser.consume(IDENTIFIER).sql;

    insertStatement->table = tableName;
        
    //pass in column name vector first
    parseColumns(insertStatement, insertStatement->columns);

    parser.consume(KEYWORD, "values");
        
    //pass in inputs vector

    //for multi-line INSERTS INSERT INTO table (a, b) VALUES (1, 2), (3, 4), (5, 6);

    do {
    
        parseInputs(insertStatement);

    }while(parser.match(SYMBOL, ","));

    parser.consume(SYMBOL, ";");

    return insertStatement;
}


void Insert::parseInputs(std::unique_ptr<InsertStatement>& insertStatement) {

    int size = 0;

    parser.consume(SYMBOL, "(");

    Select selectParser(parser);
 
    do {    
        
        std::string peekNext = parser.peek().sql;
        
        if(parser.METHODS.find(peekNext) != parser.METHODS.end()) {
           
        
            parser.consume(IDENTIFIER); 
            parser.consume(SYMBOL, "(");
                
            std::cout << "CURRENT TOKEN: " << parser.peek().sql << "\n";
            auto left = selectParser.parseExpression();
            
            auto methodExpr = std::make_unique<MethodExpression>(std::move(left));
        
            methodExpr->methodName = peekNext;
            
            insertStatement->inputs.push_back(std::move(methodExpr));            

            size++;
            
            std::cout <<  "CURRENT TOKEN" << parser.peek().sql << "\n";
            parser.consume(SYMBOL, ")");
            
        }else {
            auto insert = selectParser.parseExpression();
                
            insertStatement->inputs.push_back(std::move(insert));

            size++;
        }

    }while(parser.match(SYMBOL, ","));

    
    verifyInsert(insertStatement, size);

    parser.consume(SYMBOL, ")");

}

void Insert::parseColumns(std::unique_ptr<InsertStatement>& insertStatement, std::vector<std::string>& v) {

    parser.consume(SYMBOL, "(");

    do {
        
        Token col = parser.consume(IDENTIFIER);

        insertStatement->columns.push_back(col.sql);

    }while(parser.match(SYMBOL, ","));

    parser.consume(SYMBOL, ")");
    
}

void Insert::verifyInsert(const std::unique_ptr<InsertStatement>& insertStatement, const int& size) {

    if(insertStatement->columns.size() != size){
        std::cout << "COLUMN SIZE" << insertStatement->columns.size() << "\n";
        std::cout << size << "\n";
        throw std::runtime_error("COLUMNS AND INPUT LENGTH DON'T MATCH");
    }
}
