#pragma once

#include "Catalog.h"
#include "../../sql/ast/Node.h"
#include "../../engine/Executor.h"
#include <vector>

namespace executor {

// Result row for SELECT
struct ResultRow {
    std::vector<Value> values;
    std::vector<std::string> columnNames;
};

class SelectExecutor {
public:
    SelectExecutor(Catalog* catalog);
    
    // Execute SELECT statement and return results
    std::vector<ResultRow> execute(SelectStatement* stmt);
    
    // Print results
    void printResults(const std::vector<ResultRow>& results);
    
private:
    Catalog* catalog_;
    Executor executor_;  // For WHERE clause evaluation
    
    // Check if WHERE clause passes for current row
    bool evaluateWhere(Expression* whereClause);
    
    long long last_query_time_ms_;
};

} // namespace executor
