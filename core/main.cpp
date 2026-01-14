#include <iostream>
#include "sql/lexer/lexer.h"

int main() {

    std::cout << "Hello world" << "\n";
    std::string tests[4] = {"SELECT * FROM USERS",
        "SELECT id from users",
        "SELECT name, age FROM people",
        "INSERT INTO users VALUES (1, 'sam')"};

    for(int i = 0;i < std::size(tests);i++) {
        std::cout << "TEST: " << i + 1 << "\n";
        Lexer l;
        l.lexer(tests[i]);
        std::cout << "\n\n\n";
    }
    

    return 0;
}
