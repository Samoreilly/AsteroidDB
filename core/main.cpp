#include <iostream>
#include "sql/lexer/lexer.h"

int main() {

    std::cout << "Hello world" << "\n";

    std::string test_sql = "SELECT * FROM Users";

    Lexer l;
    l.lexer(test_sql);

    return 0;
}
