#include <iostream>
#include <vector>

class Node {
public:

    virtual ~Node();
    virtual void exec();
};

class SelectStatement : Node {
public:

    std::vector<std::string> columns;
    std::string table;
    
   void exec() override {

    }
};

class InsertStatement : Node {
public:
    

    std::vector<std::string> columns;
    std::string table;
    
    void exec() override {

    }
};

class DeleteStatement : Node {

    std::vector<std::string> columns;
    std::string table;
    
    void exec() override {

    }
};


