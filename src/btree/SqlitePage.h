//
// Created by Manoj Inukolunu on 11/10/25.
//

#ifndef SQLITEPAGE_H
#define SQLITEPAGE_H
#include <memory>

#include "../common/SqliteUtils.h"

namespace btree{
    enum class CellType { LEAF_TABLE_CELL };

    struct Cell {
        CellType cellType;
        std::uint64_t rowId;
        std::size_t payloadSize;
        std::vector<std::tuple<DataType, long, long>> dataFormat;
        long cellDataOffset;
    };


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
        std::vector<Cell> cells;

        int cellPointerArrayStart() const {
            return headerSize(pageType);
        }

        void printAllCellData(Cell cell);
        static void printId(Cell cell);
        void printColumn(Cell cell, int columnIndex) const;

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
