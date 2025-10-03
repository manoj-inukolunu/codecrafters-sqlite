//
// Created by Manoj Inukolunu on 28/09/25.
//

#include "SqliteSchemaPageReader.h"

#include <filesystem>
#include <format>
#include <vector>


SqliteSchemaPageReader::SqliteSchemaPageReader(int pageNum, int pageSize, std::ifstream& dbFile) : dbFile(dbFile),
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


int SqliteSchemaPageReader::read2Bytes(FileOffset offset) {
    FileOffset current = dbFile.tellg();
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


int SqliteSchemaPageReader::read1Byte(FileOffset offset) {
    FileOffset current = dbFile.tellg();
    dbFile.seekg(offset, std::ios::beg);
    size_t n = 1;
    uint16_t value = 0;
    dbFile.read(reinterpret_cast<char*>(&value), n);
    dbFile.seekg(current, std::ios::beg);
    return value;
}


std::pair<uint64_t, FileOffset> SqliteSchemaPageReader::readVarInt(FileOffset offset) {
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
        offset += 1;
        if (isMsbZero(currentByte)) {
            break;
        }
    }
    accum = data[0];
    for (int i = 1; i <= breakPoint; i++) {
        if (breakPoint == 8 && i == breakPoint) {
            accum = accum << 8 | data[i];
        } else {
            accum = accum << 7 | data[i];
        }
    }
    return std::make_pair(accum, offset);
}


void SqliteSchemaPageReader::printCellPointers() {
    std::cout << "Cell Content starting at " << this->cellContentAreaStart << std::endl;
    int pageHeaderSize = (this->pageType == LEAF_TABLE_PAGE || this->pageType == LEAF_INDEX_PAGE) ? 8 : 12;

    auto cellPointerStart = std::ifstream::pos_type(pageNum * pageSize).operator+(pageHeaderSize);
    if (pageNum == 0) {
        cellPointerStart += 100;
    }
    std::cout << "Number of cells in this page " << numCellsInPage << std::endl;
    std::cout << "CellPointer starting from " << cellPointerStart << std::endl;
    for (int i = 0; i < this->numCellsInPage; i++) {
        SqliteBTreeCell cell;
        cell.offset = read2Bytes(cellPointerStart);
        cell.cellNumber = i;
        cellContentOffsets.emplace_back(cell.offset);
        cellPointerStart += 2;
        cells.emplace_back(cell);
    }

    for (int i = 0; i < cellContentOffsets.size(); i++) {
        std::cout << cellContentOffsets[i] << std::endl;
    }
    processAllCells();
}

void SqliteSchemaPageReader::buildCell(SqliteBTreeCell& cell) {
    auto sizeOfRecord = readVarInt(cellContentOffsets[cell.cellNumber]);
    cell.recordSize = sizeOfRecord.first;
    auto rowId = readVarInt(sizeOfRecord.second);
    cell.rowId = rowId.first;
    SqliteBTreeCell::RecordHeader recordHeader;
    auto recordHeaderSize = readVarInt(rowId.second);
    recordHeader.headerSize = recordHeaderSize.first;

    FileOffset columnOffSet = recordHeaderSize.second;
    std::vector<RecordColumn> columns;
    for (int i = 1; i < recordHeader.headerSize; i++) {
        std::pair<uint64_t, uint64_t> payload = readVarInt(columnOffSet);
        RecordColumn c;
        c.contentSize = contentSize(payload.first);
        c.serialType = payload.first;
        c.type = dataType(payload.first);
        columns.emplace_back(c);
        columnOffSet = payload.second;
    }
    cell.record.recordColumns = std::move(columns);
}


void SqliteSchemaPageReader::readAndPrintCell(int cellNumber) {
    std::cout << "Printing Cell " << cellNumber << std::endl;
    SqliteBTreeCell cell = cells[cellNumber];
    std::cout << "Cell Offset " << cell.offset << std::endl;
    std::cout << "Cell Record Size " << cell.recordSize << std::endl;
    std::cout << "Cell Row Id " << cell.rowId << std::endl;
    std::cout << "Cell Record Body Offset " << cell.recordBodyOffset << std::endl;

    std::cout << "Cell Record header size " << cell.record.headerSize << std::endl;

    for (int i = 0; i < cell.record.recordColumns.size(); i++) {
        RecordColumn recordColumn = cell.record.recordColumns[i];
        std::cout << "Cell Record Column " << dataTypeStr(recordColumn.type) << std::endl;
        std::cout << " Content Size " << recordColumn.contentSize << std::endl;
    };


    auto header = readVarInt(cellContentOffsets[cellNumber]);
    auto rowId = readVarInt(header.second);
    auto recordHeader = readVarInt(rowId.second);
    FileOffset offset = rowId.second;
    for (int i = 1; i < recordHeader.first; i++) {
        std::pair<uint64_t, uint64_t> payload = readVarInt(offset);
        RecordColumn c;
        c.contentSize = contentSize(payload.first);
        c.serialType = payload.first;
        c.type = dataType(payload.first);
        recordDataOffsets.emplace_back(std::make_pair(payload.second, c));
        offset = payload.second;
    }
}


void SqliteSchemaPageReader::printRecordHeader() {
    // first value is size of the header itself , we don't care about that
    for (int i = 1; i < this->recordDataOffsets.size(); i++) {
        RecordColumn c = recordDataOffsets[i].second;
        std::cout << "Data Type " << dataTypeStr(c.type) << " Data Size " << c.contentSize << std::endl;
    }
}

void SqliteSchemaPageReader::processAllCells() {
    for (int i = 0; i < numCellsInPage; i++) {
        buildCell(cells[i]);
    }
}


void SqliteSchemaPageReader::printRecordContent(FileOffset recordContentStart) {
    for (int i = 0; i < recordDataOffsets.size(); i++) {
        RecordColumn c = recordDataOffsets[i].second;
        switch (c.type) {
        case TEXT: {
            std::string buffer(c.contentSize, '\0');
            dbFile.seekg(recordContentStart, std::ios::beg);
            dbFile.read(&buffer[0], c.contentSize);
            std::cout << buffer << std::endl;
            break;
        }
        default:
            throw std::invalid_argument("Not Implemented");
        }
        std::cout << recordDataOffsets[i].first << std::endl;
    }
}


void SqliteSchemaPageReader::parseHeader() {
    std::cout << "Parsing Header for page = " << pageNum << std::endl;
    auto pageBegin = FileOffset(pageNum * pageSize);
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
        throw std::runtime_error(std::format("Invalid page type {}", type));
    }

    std::cout << "BTree Page Type " << pageTypeString << std::endl;
    //Read start first free block

    this->firstFreeBlockStart = read2Bytes(pageBegin + 1);
    this->numCellsInPage = read2Bytes(pageBegin + (3));
    this->cellContentAreaStart = read2Bytes(pageBegin + (5));
    this->fragmentedFreeBytesInCellContentArea = read1Byte(pageBegin + (7));

    if (pageType == INTERIOR_TABLE_PAGE || pageType == INTERIOR_INDEX_PAGE) {
        //        this->rightMostPointer = read4Bytes(pageBegin.operator+(8));
    }
}
