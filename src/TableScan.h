//
// Created by Manoj on 14-10-2025.
//

#ifndef TABLESCAN_H
#define TABLESCAN_H

#include "btree/SqlitePage.h"
#include "common/Sqlite.h"
#include "parser/ColumnDefinition.h"
#include "parser/ParsedExpression.h"


class TableScan {
public:
    TableScan(std::string tableName, uint32_t rooPageNum, std::ifstream& stream, int pageSize)
        : pageSize(pageSize),
          tableName(tableName),
          rootPageNum(rooPageNum) {
        traverseInOrder(loadPage(stream, rootPageNum, pageSize), stream);
    }

    std::ifstream stream;
    int pageSize;
    std::string tableName;
    std::vector<Column> selectColumns;
    uint32_t rootPageNum;
    std::vector<std::unique_ptr<btree::SqlitePage>> pages;
    //This is ordered and derived from the create table sql
    std::vector<std::shared_ptr<ColumnDefinition>> tableColumns;


    void printTable(std::optional<std::shared_ptr<ParsedExpression>> whereClause) const;

    void printRow(std::optional<std::shared_ptr<ParsedExpression>> whereClause, long rowId) const;

private:
    //Traverse all the pages in order , stream is the raw file stream of the sqlite file
    void traverseInOrder(std::unique_ptr<btree::SqlitePage> rootPage, std::ifstream& stream);
    static std::unique_ptr<btree::SqlitePage> loadPage(std::ifstream& stream, int pageNum, int pageSize);
};


#endif //TABLESCAN_H
