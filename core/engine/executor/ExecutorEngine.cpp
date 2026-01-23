#include "ExecutorEngine.h"
#include <iostream>

namespace executor {

ExecutorEngine::ExecutorEngine(const std::string& db_directory) {
    catalog_ = std::make_unique<Catalog>(db_directory);
    createExecutor_ = std::make_unique<CreateExecutor>(catalog_.get());
    insertExecutor_ = std::make_unique<InsertExecutor>(catalog_.get());
    selectExecutor_ = std::make_unique<SelectExecutor>(catalog_.get());
    deleteExecutor_ = std::make_unique<DeleteExecutor>(catalog_.get());
}

ExecutorEngine::~ExecutorEngine() {
    // Unique pointers will automatically clean up
}

void ExecutorEngine::execute(Node* node) {
    if (node == nullptr) {
        std::cout << "Cannot execute null statement" << std::endl;
        return;
    }
    
    // Try to cast to specific statement types
    if (auto* createStmt = dynamic_cast<CreateStatement*>(node)) {
        createExecutor_->execute(createStmt);
    } else if (auto* insertStmt = dynamic_cast<InsertStatement*>(node)) {
        insertExecutor_->execute(insertStmt);
    } else if (auto* selectStmt = dynamic_cast<SelectStatement*>(node)) {
        auto results = selectExecutor_->execute(selectStmt);
        selectExecutor_->printResults(results);
    } else if (auto* deleteStmt = dynamic_cast<DeleteStatement*>(node)) {
        deleteExecutor_->execute(deleteStmt);
    } else {
        std::cout << "Unknown statement type" << std::endl;
    }
}

} // namespace executor
