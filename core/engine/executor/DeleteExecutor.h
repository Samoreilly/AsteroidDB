#pragma once

#include "Catalog.h"
#include "../../sql/ast/Node.h"
#include "../../engine/Executor.h"

namespace executor {

class DeleteExecutor {
public:
    DeleteExecutor(Catalog* catalog);
    
    // Execute DELETE statement
    void execute(DeleteStatement* stmt);
    
private:
    Catalog* catalog_;
    Executor executor_;  // For WHERE clause evaluation
};

} // namespace executor
