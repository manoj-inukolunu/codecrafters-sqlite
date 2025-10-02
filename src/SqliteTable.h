//
// Created by Manoj on 02-10-2025.
//

#ifndef SQLITETABLE_H
#define SQLITETABLE_H

#include <iostream>
#include <vector>

enum columType {
    TEXT,
    INTEGER, // 8 byte signed integer
    REAL,
    BLOB
};


class SqliteTable {
public:
    SqliteTable();
    std::string tableName;
    int columnCount;
    std::vector<std::string> columnNames;
    std::vector<columType> columnTypes;
    ~SqliteTable();

private:
};


#endif //SQLITETABLE_H
