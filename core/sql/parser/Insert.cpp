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
    parseInputs(insertStatement, insertStatement->inputs);


    parser.consume(SYMBOL, ";");

    verifyInsert(insertStatement);

    return insertStatement;
}


void Insert::parseInputs(std::unique_ptr<InsertStatement>& insertStatement, std::vector<std::unique_ptr<Expression>>& inputs) {


    parser.consume(SYMBOL, "(");

    Select selectParser(parser);
 
    do {    
        
        auto insert = selectParser.parseExpression();
            
        insertStatement->inputs.push_back(std::move(insert));


    }while(parser.match(SYMBOL, ","));

    
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

void Insert::verifyInsert(const std::unique_ptr<InsertStatement>& insertStatement) {

    if(insertStatement->columns.size() != insertStatement->inputs.size()){
        throw std::runtime_error("COLUMNS AND INPUT LENGTH DON'T MATCH");
    }
}
