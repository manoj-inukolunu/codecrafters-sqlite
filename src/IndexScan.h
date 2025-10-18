//
// Created by Manoj Inukolunu on 16/10/25.
//

#ifndef INDEXSCAN_H
#define INDEXSCAN_H
#include <string>
#include <utility>

#include "parser/ColumnDefinition.h"


class IndexScan {
public:
    int pageSize;
    int rootPageNum;
    std::string dbPath;
    std::vector<std::shared_ptr<ColumnDefinition>> columns;

    IndexScan(std::string  dbPath, int pageSize, int rootPageNum, std::vector<std::shared_ptr<ColumnDefinition>> columns)
        : dbPath(std::move(dbPath)),
          columns(std::move(columns)),
          rootPageNum(rootPageNum),
          pageSize(pageSize) {
    }

    std::vector<long> findRowId(std::string keyToSearchFor);
};


#endif //INDEXSCAN_H
