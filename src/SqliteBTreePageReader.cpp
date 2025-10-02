//
// Created by Manoj Inukolunu on 28/09/25.
//

#include "SqliteBTreePageReader.h"

#include <format>
#include <vector>

SqliteBTreePageReader::SqliteBTreePageReader(int pageNum, int pageSize, std::ifstream& dbFile) : dbFile(dbFile),
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
    dbFile.read(reinterpret_cast<char*>(&value), n);
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
    dbFile.read(reinterpret_cast<char*>(&value), n);
    dbFile.seekg(current, std::ios::beg);
    return value;
}


uint64_t SqliteBTreePageReader::readVarInt(std::ifstream::pos_type offset) {
    auto isMsbZero = [](int val) {
        return ((val >> 7) & 0x01) == 0;
    };
    uint64_t accum = 0;
    uint8_t data[9];
    int breakPoint = 0;
    for (int i = 0; i < 9; i++) {
        int currentByte = read1Byte(offset);
        data[i] = (i == 8) ? currentByte : (currentByte & 0x7F);
        breakPoint = i;
        if (isMsbZero(currentByte)) {
            break;
        }
        offset += 1;
    }
    accum = data[0];
    for (int i = 1; i <= breakPoint; i++) {
        if (breakPoint == 8 && i == breakPoint) {
            accum = accum << 8 | data[i];
        } else {
            accum = accum << 7 | data[i];
        }
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
        cellContentOffsets.emplace_back(read2Bytes(cellPointerStart));
        cellPointerStart += 2;
    }

    for (int i = 0; i < cellContentOffsets.size(); i++) {
        std::cout << cellContentOffsets[i] << std::endl;
    }
}

void SqliteBTreePageReader::readAndPrintCell(int cellNumber) {

    readVarInt(cellContentOffsets[cellNumber]);
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
