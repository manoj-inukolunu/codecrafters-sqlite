//
// Created by Manoj Inukolunu on 08/10/25.
//

#ifndef SQLITE_STARTER_CPP_SQLSTATEMENT_H
#define SQLITE_STARTER_CPP_SQLSTATEMENT_H


#include "parser.h"

class SqlStatement {
public:
    explicit SqlStatement(StatementType type): statementType(type) {
    }

    virtual ~SqlStatement() = default;

    StatementType statementType;
};


#endif //SQLITE_STARTER_CPP_SQLSTATEMENT_H
