//
// Created by Manoj on 11-10-2025.
//

#ifndef SQLITEPAGE_H
#define SQLITEPAGE_H
#include "common/SqliteUtils.h"

/*
 * Leaf Page Cell
 *  VarInt - Number of Bytes of the payload   - totalSize
 *  VarInt - RowId
 *  Byte Array - Payload (Record Format)
 *      Header
 *          Size VarInt - Number of bytes in the header including this varint
 *          One or more additional varints, one per column - Serial Type
 *
 *
 *
 *
 *
 *  4 Byte Integer - Page Number of the first overflow page
 *  4 Byte Integer - Number of bytes of data on this pageq
 *
 */

class SqlitePage {
public:
    int pageNum;
    int pageSize;
    std::unique_ptr<std::uint8_t[]> data;
    std::unique_ptr<std::uint16_t[]> cellPointerArray;
    BTreePageType pageType;
    int firstFreeBlockStart;
    int numCellsInPage;
    int fragmentedFreeBytesInCellContentArea;
    int cellContentAreaStart;

    struct Cell {

    };

private:
    int headerSize;

    void parseHeader();
    void processCells();
};


#endif //SQLITEPAGE_H
