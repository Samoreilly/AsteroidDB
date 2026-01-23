#pragma once

#include "Catalog.h"
#include "CreateExecutor.h"
#include "InsertExecutor.h"
#include "SelectExecutor.h"
#include "DeleteExecutor.h"
#include "../../sql/ast/Node.h"
#include <memory>

namespace executor {

class ExecutorEngine {
public:
    ExecutorEngine(const std::string& db_directory = ".");
    ~ExecutorEngine();
    
    // Execute any statement
    void execute(Node* node);
    
    // Get catalog
    Catalog* getCatalog() { return catalog_.get(); }
    
private:
    std::unique_ptr<Catalog> catalog_;
    std::unique_ptr<CreateExecutor> createExecutor_;
    std::unique_ptr<InsertExecutor> insertExecutor_;
    std::unique_ptr<SelectExecutor> selectExecutor_;
    std::unique_ptr<DeleteExecutor> deleteExecutor_;
};

} // namespace executor
