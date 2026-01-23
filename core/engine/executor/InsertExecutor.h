#pragma once

#include "Catalog.h"
#include "../../sql/ast/Node.h"
#include "../../engine/Executor.h"

namespace executor {

class InsertExecutor {
public:
    InsertExecutor(Catalog* catalog);
    
    // Execute INSERT statement
    void execute(InsertStatement* stmt);
    
private:
    Catalog* catalog_;
    Executor executor_;  // For expression evaluation
};

} // namespace executor
