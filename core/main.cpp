#include <iostream>
#include "sql/lexer/lexer.h"

int main() {

    std::cout << "Hello world" << "\n";
    std::string tests[7] = {
        "SELECT * FROM USERS",
        "SELECT id from users",
        "SELECT name, age FROM people",
        "INSERT INTO users VALUES (1, 'sam')",
        "SELECT first_name, last_name, salary FROM employees;",
        "SELECT * FROM employees WHERE department = 'Sales';",
        "INSERT INTO employees (first_name, last_name, department, salary) VALUES ('Alice', 'Smith', 'Marketing', 60000);"
    };

    for(int i = 0;i < std::size(tests);i++) {
        std::cout << "TEST: " << i + 1 << "\n";
        std::cout << tests[i] << "\n\n\n";

        Lexer l;
        l.lexer(tests[i]);
        std::cout << "\n\n\n";
    }
    

    return 0;
}
