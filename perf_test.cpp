#include "core/engine/executor/ExecutorEngine.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <filesystem>

using namespace executor;

void runPerfTest() {
    std::cout << "=== AsteroidDB B+ Tree Performance Test ===" << std::endl;

    // Clean up
    std::filesystem::remove("big_table.db");
    std::filesystem::remove("catalog.meta");

    ExecutorEngine engine(".");
    
    // 1. Create table with index on 'id'
    std::cout << "Creating table 'big_table' (id INT, val VARCHAR)..." << std::endl;
    auto createStmt = std::make_unique<CreateStatement>();
    createStmt->table = "big_table";
    
    CreateColumn c1; c1.name = "id"; c1.type = "INT";
    createStmt->columns.push_back(std::move(c1));
    
    CreateColumn c2; c2.name = "val"; c2.type = "VARCHAR";
    createStmt->columns.push_back(std::move(c2));
    
    engine.execute(createStmt.get());

    // 2. Insert 5000 rows
    std::cout << "Inserting 5,000 rows..." << std::endl;
    auto start_insert = std::chrono::high_resolution_clock::now();
    for (int i = 1; i <= 5000; i++) {
        auto insertStmt = std::make_unique<InsertStatement>();
        insertStmt->table = "big_table";
        insertStmt->columns = {"id", "val"};
        insertStmt->inputs.push_back(std::make_unique<Literal>(Value(i)));
        insertStmt->inputs.push_back(std::make_unique<Literal>(Value("row_" + std::to_string(i))));
        engine.execute(insertStmt.get());
    }
    auto end_insert = std::chrono::high_resolution_clock::now();
    auto insert_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_insert - start_insert).count();
    std::cout << "Insert of 5,000 rows took " << insert_ms << " ms" << std::endl;

    // 3. Select with Index Seek
    std::cout << "\nRunning Index Seek: SELECT * FROM big_table WHERE id = 4500" << std::endl;
    auto selectIndex = std::make_unique<SelectStatement>();
    selectIndex->table = "big_table";
    selectIndex->columns = {"*"};
    
    auto idExpr = std::make_unique<Identifier>("id");
    auto valExpr = std::make_unique<Literal>(Value(4500));
    selectIndex->whereClause = std::make_unique<BinaryExpression>(
        std::move(idExpr), "=", std::move(valExpr)
    );
    
    engine.execute(selectIndex.get());

    // 4. Force Full Scan with a condition that doesn't match the index seek pattern
    // (e.g., identity comparison or just a query on a column without index)
    std::cout << "\nRunning Full Scan: SELECT * FROM big_table WHERE val = 'row_4500'" << std::endl;
    auto selectFull = std::make_unique<SelectStatement>();
    selectFull->table = "big_table";
    selectFull->columns = {"*"};
    
    auto valExpr2 = std::make_unique<Identifier>("val");
    auto targetExpr = std::make_unique<Literal>(Value("row_4500"));
    selectFull->whereClause = std::make_unique<BinaryExpression>(
        std::move(valExpr2), "=", std::move(targetExpr)
    );
    
    engine.execute(selectFull.get());

    std::cout << "\n=== Performance Test Complete ===" << std::endl;
}

int main() {
    try {
        runPerfTest();
    } catch (const std::exception& e) {
        std::cerr << "Fail: " << e.what() << std::endl;
    }
    return 0;
}
