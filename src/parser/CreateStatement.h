//
// Created by Manoj Inukolunu on 08/10/25.
//

#ifndef SQLITE_STARTER_CPP_CREATESTATEMENT_H
#define SQLITE_STARTER_CPP_CREATESTATEMENT_H

#include "SqlStatement.h"
#include "../Sqlite.h"

class CreateStatement : SqlStatement {
public:
    CreateStatement(StatementType type) : SqlStatement(type) {

    }

    std::string tableName;

};

#endif //SQLITE_STARTER_CPP_CREATESTATEMENT_H
