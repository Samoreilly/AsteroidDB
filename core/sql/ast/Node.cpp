#include "Node.h"
#include "Expression.h"
#include <iostream>

Node::~Node() {}

SelectStatement::~SelectStatement() = default;

void SelectStatement::print() const {
    std::cout << "SelectStatement {" << std::endl;
    std::cout << "  columns: [";
    for (size_t i = 0; i < columns.size(); i++) {
        std::cout << columns[i];
        if (i < columns.size() - 1) std::cout << ", ";
    }
    std::cout << "]" << std::endl;
    std::cout << "  table: " << table << std::endl;
    std::cout << "  whereClause: ";
    if (whereClause) {
        std::cout << std::endl;
        whereClause->print(2);  // Pass indent level
    } else {
        std::cout << "null" << std::endl;
    }
    std::cout << "}" << std::endl;
}

CreateStatement::~CreateStatement() = default;



void CreateStatement::print() const {
    std::cout << "CreateStatement {" << std::endl;
    if (!database.empty()) {
        std::cout << "  database: " << database << std::endl;
    }
    std::cout << "  table: " << table << std::endl;
    std::cout << "  columns: [" << std::endl;
    for (size_t i = 0; i < columns.size(); ++i) {
        const auto& col = columns[i];
        std::cout << "    " << col.name << " " << col.type;
        if (!col.constraints.empty()) {
            std::cout << " (";
            for (size_t j = 0; j < col.constraints.size(); ++j) {
                std::cout << col.constraints[j];
                if (j < col.constraints.size() - 1) std::cout << ", ";
            }
            std::cout << ")";
        }

        // Print the check expression if it exists
        if (col.check) {
            std::cout << " CHECK ";
            col.check->print(6);  // indent a bit for readability
        }

        if (i < columns.size() - 1)
            std::cout << ",";
        std::cout << std::endl;
    }
    std::cout << "  ]" << std::endl;

    if (!foreignKeys.empty()) {
        std::cout << "  foreignKeys: [" << std::endl;
        for (size_t i = 0; i < foreignKeys.size(); ++i) {
            const auto& fk = foreignKeys[i];

            if (!fk.constraintName.empty()) {
                std::cout << "CONSTRAINT " << fk.constraintName << " ";
            }

            std::cout << "    FOREIGN KEY (";
            for (size_t j = 0; j < fk.columnNames.size(); ++j) {
                std::cout << fk.columnNames[j];
                if (j < fk.columnNames.size() - 1) std::cout << ", ";
            }
            std::cout << ") REFERENCES " << fk.referencedTable << " (";
            for (size_t j = 0; j < fk.referencedColumns.size(); ++j) {
                std::cout << fk.referencedColumns[j];
                if (j < fk.referencedColumns.size() - 1) std::cout << ", ";
            }
            std::cout << ")";
            if (i < foreignKeys.size() - 1)
                std::cout << ",";
            std::cout << std::endl;
        }
        std::cout << "  ]" << std::endl;
    }

    std::cout << "}" << std::endl;
}

