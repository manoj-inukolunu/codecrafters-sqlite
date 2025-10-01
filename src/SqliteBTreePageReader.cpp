//
// Created by Manoj Inukolunu on 28/09/25.
//

#include "SqliteBTreePageReader.h"


SqliteBTreePageReader::SqliteBTreePageReader(int pageNum, int pageSize, std::ifstream &dbFile) : dbFile(dbFile),
                                                                                                 pageNum(pageNum),
                                                                                                 pageSize(pageSize) {
    std::cout << "Reading page = " << pageNum << " with pageSize = " << pageSize << std::endl;
    if (pageNum == 1) {
        std::cout << "This is the first page There is a 100 byte header for this page" << std::endl;
        this->parseHeader();
    } else {
        this->parseHeader();
    }
}


int SqliteBTreePageReader::read2Bytes(std::ifstream::pos_type offset) {
    std::ifstream::pos_type current = dbFile.tellg();
    dbFile.seekg(offset, std::ios::beg);
    size_t n = 2;
    uint16_t value = 0;
    dbFile.read(reinterpret_cast<char *>(&value), n);
    if (little_endian()) {
        value = swap(value);
    }
    dbFile.seekg(current, std::ios::beg);
    return value;
}


int SqliteBTreePageReader::read1Byte(std::ifstream::pos_type offset) {
    std::ifstream::pos_type current = dbFile.tellg();
    dbFile.seekg(offset, std::ios::beg);
    size_t n = 1;
    uint16_t value = 0;
    dbFile.read(reinterpret_cast<char *>(&value), n);
    dbFile.seekg(current, std::ios::beg);
    return value;
}


long SqliteBTreePageReader::readVarInt(std::ifstream::pos_type offset) {

    auto isMsbZero = [](int val) {
        return ((val >> 7) & 0x01) == 0;
    };
    int count = 1, accum = read1Byte(offset);
    if (isMsbZero(accum)) {
        return accum;
    }
    offset+=1;
    while (count <= 8) {
        int curr = read1Byte(offset);
        if (isMsbZero(curr)) {
            accum = (accum << 7) | (curr << 1);
            return accum;
        } else {
            accum = (accum << 7) | (curr << 1);
            offset += 1;
        }

        count++;
    }
    return accum;
}


void SqliteBTreePageReader::printCellPointers() {
    std::cout << "Cell Content starting at " << this->cellContentAreaStart << std::endl;
    int pageHeaderSize = (this->pageType == LEAF_TABLE_PAGE || this->pageType == LEAF_INDEX_PAGE) ? 8 : 12;

    auto cellPointerStart = std::ifstream::pos_type(pageNum * pageSize).operator+(pageHeaderSize);
    if (pageNum == 0) {
        cellPointerStart += 100;
    }
    std::cout << "Number of cells in this page " << numCellsInPage << std::endl;
    std::cout << "CellPointer starting from " << cellPointerStart << std::endl;
    for (int i = 0; i < this->numCellsInPage; i++) {
        std::cout << read2Bytes(cellPointerStart) << std::endl;
        cellPointerStart += 2;
    }

}

void SqliteBTreePageReader::parseHeader() {
    std::cout << "Parsing Header for page = " << pageNum << std::endl;
    auto pageBegin = std::ifstream::pos_type(pageNum * pageSize);
    if (pageNum == 0) {
        pageBegin += 100;
    }
    // Read page type
    int type = read1Byte(pageBegin);
    std::string pageTypeString;
    switch (type) {
        case 2 :
            pageType = INTERIOR_INDEX_PAGE;
            pageTypeString = "INTERIOR_INDEX_PAGE";
            break;
        case 5 :
            pageType = INTERIOR_TABLE_PAGE;
            pageTypeString = "INTERIOR_TABLE_PAGE";
            break;
        case 10 :
            pageType = LEAF_INDEX_PAGE;
            pageTypeString = "LEAF_INDEX_PAGE";
            break;
        case 13 :
            pageType = LEAF_TABLE_PAGE;
            pageTypeString = "LEAF_TABLE_PAGE";
            break;
        default:
            pageTypeString = "UNKNOWN";
            throw std::runtime_error(std::format("Invalid page type {}", type));
    }

    std::cout << "BTree Page Type " << pageTypeString << std::endl;
    //Read start first free block

    this->firstFreeBlockStart = read2Bytes(pageBegin.operator+(1));
    this->numCellsInPage = read2Bytes(pageBegin.operator+(3));
    this->cellContentAreaStart = read2Bytes(pageBegin.operator+(5));
    this->fragmentedFreeBytesInCellContentArea = read1Byte(pageBegin.operator+(7));

    if (pageType == INTERIOR_TABLE_PAGE || pageType == INTERIOR_INDEX_PAGE) {
//        this->rightMostPointer = read4Bytes(pageBegin.operator+(8));
    }
}