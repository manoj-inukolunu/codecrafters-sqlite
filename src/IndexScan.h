//
// Created by Manoj Inukolunu on 16/10/25.
//

#ifndef INDEXSCAN_H
#define INDEXSCAN_H
#include <string>

#include "parser/ColumnDefinition.h"


class IndexScan {
public:
    int pageSize;
    int rootPageNum;
    std::vector<std::shared_ptr<ColumnDefinition>> columns;

    IndexScan(int pageSize, int rootPageNum, std::vector<std::shared_ptr<ColumnDefinition>> columns): columns(columns),
                                                                                                      rootPageNum(rootPageNum),
                                                                                                      pageSize(pageSize) {
    }

    long findRowId(std::string keyToSearchFor);
};


#endif //INDEXSCAN_H
