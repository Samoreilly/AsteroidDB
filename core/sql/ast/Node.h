#include <iostream>
#include <vector>

class Node {
public:

    virtual ~Node();
    virtual void exec() = 0;
};

class SelectStatement : public Node {
public:

    //select certain fields from a table
    std::vector<std::string> columns;
    std::string table;
    
    void exec() override {
        std::cout << "Executing select from TABLE: " << table << "\n";
    }
};

class InsertStatement : public Node {
public:
    

    std::vector<std::string> columns;
    std::string table;
    
    void exec() override {

    }
};

class DeleteStatement : public Node {
public:

    std::vector<std::string> columns;
    std::string table;
    
    void exec() override {

    }
};

class CreateStatement : public Node {
public:

    std::vector<std::string> columns;
    std::string table;

    void exec() override {

    }
};

