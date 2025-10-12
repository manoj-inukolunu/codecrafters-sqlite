//
// Created by Manoj Inukolunu on 11/10/25.
//

#ifndef SQLITEPAGE_H
#define SQLITEPAGE_H
#include <memory>

#include "../common/Sqlite.h"
#include "../common/SqliteUtils.h"

namespace btree{
    struct RowCell {
        std::unique_ptr<Column> column;
        std::unique_ptr<uint8_t[]> data;
    };

    struct Row {
        std::vector<std::unique_ptr<RowCell>> cells;
    };

    class SqlitePage {
    public:

    public:
        int pageSize;
        int pageNum;
        std::unique_ptr<uint8_t[]> data;

    private:
    };
}


#endif //SQLITEPAGE_H
