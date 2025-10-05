//
// Created by Manoj Inukolunu on 04/10/25.
//

#ifndef SQLITE_STARTER_CPP_SQLITEPAGEREADER_H
#define SQLITE_STARTER_CPP_SQLITEPAGEREADER_H

#include <vector>

#include "SqliteUtils.h"

struct SqlitePageCell {

};

class SqlitePageReader {
    int pageNum;
    int pageSize;
    BTreePageType pageType;
    int numCellsInPage;
    FileOffset cellContentAreaStart;
    std::vector<SqlitePageCell> cells;

};


#endif //SQLITE_STARTER_CPP_SQLITEPAGEREADER_H
