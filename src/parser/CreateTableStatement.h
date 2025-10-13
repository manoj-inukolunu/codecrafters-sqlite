//
// Created by Manoj Inukolunu on 08/10/25.
//

#ifndef SQLITE_STARTER_CPP_CREATESTATEMENT_H
#define SQLITE_STARTER_CPP_CREATESTATEMENT_H

#include <optional>

#include "ColumnDefinition.h"
#include "SqlStatement.h"
#include "../common/Sqlite.h"

class CreateTableStatement : SqlStatement {
public:
    explicit CreateTableStatement(StatementType type) : SqlStatement(type) {
    }

    std::string tableName;
    std::optional<bool> isTemporary;
    std::optional<bool> ifNotExists;
    std::optional<std::string> schemaName;
    // NO Support now
    std::optional<bool> isCreateAsSelect;
    std::vector<std::shared_ptr<ColumnDefinition>> columns;
    std::optional<bool> withoutRowId;
    std::vector<std::shared_ptr<Constraint>> tableConstraints;
};

#endif //SQLITE_STARTER_CPP_CREATESTATEMENT_H
