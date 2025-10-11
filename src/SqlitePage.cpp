//
// Created by Manoj on 11-10-2025.
//

#include "SqlitePage.h"

void SqlitePage::parseHeader() {
    auto pageBegin = 0;
    if (pageNum == 1) {
        pageBegin += 100;
    }
    int type = read1Byte(pageSize, pageBegin, data);
    std::string pageTypeString;
    switch (type) {
    case 2:
        pageType = INTERIOR_INDEX_PAGE;
        pageTypeString = "INTERIOR_INDEX_PAGE";
        break;
    case 5:
        pageType = INTERIOR_TABLE_PAGE;
        pageTypeString = "INTERIOR_TABLE_PAGE";
        break;
    case 10:
        pageType = LEAF_INDEX_PAGE;
        pageTypeString = "LEAF_INDEX_PAGE";
        break;
    case 13:
        pageType = LEAF_TABLE_PAGE;
        pageTypeString = "LEAF_TABLE_PAGE";
        break;
    default:
        throw std::runtime_error(std::format("Invalid page type {}", type));
    }

    this->firstFreeBlockStart = read2Bytes(pageSize, pageBegin + 1, data);
    this->numCellsInPage = read2Bytes(pageSize, pageBegin + (3), data);
    this->cellContentAreaStart = read2Bytes(pageSize, pageBegin + (5), data);
    this->fragmentedFreeBytesInCellContentArea = read1Byte(pageSize, pageBegin + (7), data);
    this->headerSize = (this->pageType == LEAF_TABLE_PAGE || this->pageType == LEAF_INDEX_PAGE) ? 8 : 12;


    if (pageType == INTERIOR_TABLE_PAGE || pageType == INTERIOR_INDEX_PAGE) {
        //        this->rightMostPointer = read4Bytes(pageBegin.operator+(8));
    }
}

void SqlitePage::processCells() {
}
