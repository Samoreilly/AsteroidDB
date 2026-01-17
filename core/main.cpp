
#include <iostream>
#include "sql/lexer/lexer.h"
#include "sql/ast/Parser.h"

int main() {

    std::cout << "Hello world" << "\n";
    std::string tests[22] = {
        "SELECT * FROM USERS",
        "SELECT id from users",
        "SELECT name, age FROM people",
        "INSERT INTO users VALUES (1, 'sam')",
        "SELECT first_name, last_name, salary FROM employees;",
        "SELECT * FROM employees WHERE department = 'Sales';",
        "INSERT INTO employees (first_name, last_name, department, salary) VALUES ('Alice', 'Smith', 'Marketing', 60000);",
        "SELECT * FROM users WHERE age > 18",
        "SELECT * FROM users WHERE age >= 18 AND salary <= 50000",
        "SELECT * FROM products WHERE price <> 0",
        "SELECT   *    FROM     users",
        "SELECT * FROM users WHERE name='John'",
        "SELECT * FROM products WHERE price = 19.99",
        "UPDATE accounts SET balance = -100.50",
        "INSERT INTO measurements VALUES (3.14159, -273.15, 0)",
        "INSERT INTO users VALUES ('', 'empty')",
        "INSERT INTO employees VALUES ('John Doe', 'New York')",
        "SELECT * FROM users WHERE age > 18 OR status = 'active'",
        "SELECT * FROM orders WHERE total >= 100 AND status != 'cancelled'",   
        "SELECT * FROM users WHERE (age > 18 AND country = 'US') OR (age > 21 AND country = 'UK')",
        "UPDATE users SET name = 'Bob', age = 30 WHERE id = 1",
        "DELETE FROM users WHERE id = 5" 
    };

    for(int i = 0;i < std::size(tests);i++) {
        //std::cout << "TEST: " << i + 1 << "\n";
        //std::cout << tests[i] << "\n\n\n";

      //  Lexer l;

    //    l.lexer(tests[i]);
        //std::cout << "\n\n\n";
    }

    try {
        Lexer lexer;
        lexer.lexer("SELECT id, email FROM customers WHERE active = true");
        
        std::cout << "\nselect name, age FROM users WHERE age > 18\n\n"; 
        
        Parser parser(lexer.getTokens());
        std::unique_ptr<Node> ast = parser.parse();
        
        if(auto* selectStmt = dynamic_cast<SelectStatement*>(ast.get())) {

            selectStmt->print();
        }
        

        std::cout << "\nAbstract syntax tree built" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error-> " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
