//
// Created by Manoj Inukolunu on 06/10/25.
//

#ifndef SQLITE_STARTER_CPP_SQLITE_H
#define SQLITE_STARTER_CPP_SQLITE_H


#include "SqliteUtils.h"

struct Column {
    DataType type;
    std::string name;
};

enum SqliteObjectType {
    TABLE, INDEX, TRIGGER
};

struct Table {
    std::string tableName;
    std::vector<Column> columns;
};

#endif //SQLITE_STARTER_CPP_SQLITE_H
