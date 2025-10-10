//
// Created by Manoj on 09-10-2025.
//

#ifndef SELECTSTATEMENT_H
#define SELECTSTATEMENT_H


#include <optional>

#include "SqlStatement.h"
#include "ParsedExpression.h"
#include "../common/Sqlite.h"


class SelectStatement : public SqlStatement {
public:
    explicit SelectStatement(StatementType type) : SqlStatement(type) {
    }

    std::optional<bool> distinct;
    std::optional<bool> all;

    std::shared_ptr<Table> fromTable;

    std::optional<std::shared_ptr<ParsedExpression>> whereClause;
    std::optional<ParsedExpression> havingClause;
    std::vector<ParsedExpression> groups;
};


#endif //SELECTSTATEMENT_H
