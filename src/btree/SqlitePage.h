//
// Created by Manoj Inukolunu on 11/10/25.
//

#ifndef SQLITEPAGE_H
#define SQLITEPAGE_H
#include <memory>

#include "../common/SqliteUtils.h"

namespace btree{
    class SqlitePage {
    public:
        SqlitePage(int size, int number, std::unique_ptr<std::uint8_t[]> buffer)
            : pageSize(size),
              pageNum(number),
              data(std::move(buffer)) {
            parseHeader();
        }

        int pageSize;
        int pageNum;
        std::unique_ptr<std::uint8_t[]> data;
        BTreePageType pageType;
        int firstFreeBlockStart;
        int numCellsInPage;
        int cellContentAreaStart;
        int fragmentedFreeBytesInCellContentArea;
        std::vector<long> cellContentOffsets;

        int cellPointerArrayStart() const {
            return headerSize(pageType);
        }

    private:
        static constexpr int headerSize(BTreePageType t) noexcept {
            return t == LEAF_INDEX_PAGE || t == LEAF_TABLE_PAGE ? 8 : 12;
        }

        void processCellPointers();
        void buildCell(int cellNumber);
        void processAllCells();
        void parseHeader();
    };
}


#endif //SQLITEPAGE_H
