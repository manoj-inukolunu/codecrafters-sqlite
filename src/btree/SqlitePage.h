//
// Created by Manoj Inukolunu on 11/10/25.
//

#ifndef SQLITEPAGE_H
#define SQLITEPAGE_H
#include <memory>

#include "../common/SqliteUtils.h"

namespace btree{
    enum class CellType { LEAF_TABLE_CELL, INTERIOR_TABLE_CELL };

    struct Cell {
        CellType cellType;
        // The largest rowid that appears in the left child page. Used for navigation only — not an actual record if the cellType is INTERIOR_TABLE_CELL
        varint rowId;
        //only for cellType = INTERIOR_TABLE_CELL
        std::optional<uint32_t> leftChildPageNum;
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
        uint32_t pageNum;
        std::unique_ptr<std::uint8_t[]> data;
        BTreePageType pageType;
        int firstFreeBlockStart;
        int numCellsInPage;
        int cellContentAreaStart;
        int fragmentedFreeBytesInCellContentArea;
        std::vector<long> cellContentOffsets;
        std::vector<Cell> cells;

        // rightMostChildPageNum are only available for interior table b tree pages
        uint32_t rightMostChildPageNum;


        int cellPointerArrayStart() const {
            return headerSize(pageType);
        }

        void printAllCellData(Cell cell);
        static void printId(Cell cell);
        std::vector<std::string> collectColumnData(Cell cell) const;

    private:
        static constexpr int headerSize(BTreePageType t) noexcept {
            return t == LEAF_INDEX_PAGE || t == LEAF_TABLE_PAGE ? 8 : 12;
        }

        void processCellPointers();
        void buildLeafCell(int cellNumber);
        void buildInteriorCell(int cellNumber);
        void processAllCells();
        void parseHeader();
    };
}


#endif //SQLITEPAGE_H
