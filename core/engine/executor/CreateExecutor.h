#pragma once

#include "Catalog.h"
#include "../../sql/ast/Node.h"

namespace executor {

class CreateExecutor {
public:
    CreateExecutor(Catalog* catalog);
    
    // Execute CREATE TABLE statement
    void execute(CreateStatement* stmt);
    
private:
    Catalog* catalog_;
};

} // namespace executor
