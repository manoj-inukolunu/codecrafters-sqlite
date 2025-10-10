//
// Created by Manoj on 07-10-2025.
//

#ifndef CREATE_H
#define CREATE_H
#include <string>
#include <memory>

#endif //CREATE_H

enum class TableReferenceType {
    BASE_TABLE,
    JOIN,
    SUBQUERY,
    CTE
};

enum class StatementType {
    SELECT_STATEMENT,
    CREATE_STATEMENT
};


const static std::string statementTypeToString(StatementType type) {
    switch (type) {
    case StatementType::SELECT_STATEMENT:
        return "SELECT_STATEMENT";
    case StatementType::CREATE_STATEMENT:
        return "CREATE_STATEMENT";
    default:
        return "UNKNOWN_STATEMENT";
    }
}
