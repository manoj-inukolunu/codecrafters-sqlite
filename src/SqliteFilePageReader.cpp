//
// Created by Manoj Inukolunu on 28/09/25.
//

#include "SqliteFilePageReader.h"

#include <filesystem>
#include <format>
#include <vector>

#include "ANTLRInputStream.h"
#include "SQLiteLexer.h"
#include "SQLiteParser.h"
#include "SQLiteAstBuilder.hpp"


SqliteFilePageReader::SqliteFilePageReader(int pageNum, const std::string& filePath) : pageNum(pageNum),
                                                                                       filePath(filePath) {
    if (pageNum == 0) {
        throw std::runtime_error("Invalid Page Number 0");
    }
    dbFile = std::make_unique<std::ifstream>(filePath, std::ios::binary);
    pageSize = readPageSize();
    loadPage(1);
    pageBegin = 0;
    pageSize = read2Bytes(pageSize, pageBegin + 16, pageCache[0]);
    pageBegin = static_cast<FileOffset>((pageNum - 1) * pageSize);
    parseHeader();
}


uint16_t SqliteFilePageReader::readPageSize() {
    if (!dbFile)
        throw std::runtime_error("Cannot open SQLite file: " + filePath);

    // Seek to offset 16
    dbFile->seekg(16, std::ios::beg);
    if (!dbFile)
        throw std::runtime_error("Failed to seek to offset 16");

    // Read 2 bytes
    std::uint8_t bytes[2];
    dbFile->read(reinterpret_cast<char*>(bytes), 2);
    if (dbFile->gcount() != 2)
        throw std::runtime_error("Short read while reading page size");

    // Combine as big-endian
    uint16_t pageSize = (static_cast<uint16_t>(bytes[0]) << 8) |
        static_cast<uint16_t>(bytes[1]);

    // SQLite special case: value 1 means 65536
    if (pageSize == 1)
        pageSize = 65536;

    return pageSize;
}


void SqliteFilePageReader::processCellPointers() {
    int pageHeaderSize = (this->pageType == LEAF_TABLE_PAGE || this->pageType == LEAF_INDEX_PAGE) ? 8 : 12;

    auto cellPointerStart = std::ifstream::pos_type((pageNum - 1) * pageSize).operator+(pageHeaderSize);
    if (pageNum == 1) {
        cellPointerStart += 100;
    }
    for (int i = 0; i < this->numCellsInPage; i++) {
        SqliteBTreeSchemaCell cell;
        cell.offset = pageBegin + read2Bytes(pageSize, cellPointerStart, pageCache[pageNum - 1]);
        cell.cellNumber = i;
        cellContentOffsets.emplace_back(cell.offset);
        cellPointerStart += 2;
        cells.emplace_back(cell);
    }
    processAllCells();
}

void SqliteFilePageReader::buildCell(SqliteBTreeSchemaCell& cell) {
    auto sizeOfRecord = readVarInt(pageSize, cellContentOffsets[cell.cellNumber], pageCache[pageNum - 1]);
    cell.recordSize = sizeOfRecord.first;
    auto rowId = readVarInt(pageSize, sizeOfRecord.second, pageCache[pageNum - 1]);
    cell.rowId = rowId.first;
    SqliteBTreeSchemaCell::RecordHeader recordHeader;
    auto recordHeaderSize = readVarInt(pageSize, rowId.second, pageCache[pageNum - 1]);
    recordHeader.headerSize = recordHeaderSize.first;

    FileOffset columnOffSet = recordHeaderSize.second;
    std::vector<RecordColumn> columns;
    long totalsize = recordHeader.headerSize;
    while (totalsize < cell.recordSize) {
        std::pair<uint64_t, FileOffset> payload = readVarInt(pageSize, columnOffSet, pageCache[pageNum - 1]);
        RecordColumn c{};
        c.contentSize = contentSize(payload.first);
        totalsize += c.contentSize;
        c.serialType = payload.first;
        c.type = dataType(payload.first);
        c.numBits = numBits(c.serialType);
        columns.emplace_back(c);
        columnOffSet = payload.second;
    }
    cell.recordBodyOffset = columnOffSet;
    cell.record.recordColumns = std::move(columns);

    assert(totalsize == sizeOfRecord.first);
    buildCellBody(cell);
}

void SqliteFilePageReader::buildCellBody(SqliteBTreeSchemaCell& cell) {
    FileOffset offset = cell.recordBodyOffset;
    for (auto& recordColumn : cell.record.recordColumns) {
        switch (recordColumn.type) {
        case TEXT: {
            std::unique_ptr<char[]> buffer(new char[recordColumn.contentSize]());
            dbFile->seekg(offset, std::ios::beg);
            dbFile->read(buffer.get(), recordColumn.contentSize);
            // Construct string from raw bytes
            std::string result(buffer.get(), recordColumn.contentSize);
            recordColumn.content.assign(buffer.get(), recordColumn.contentSize);
            break;
        }
        case INT: {
            dbFile->seekg(offset, std::ios::beg);
            int64_t value = 0;
            dbFile->read(reinterpret_cast<char*>(&value), recordColumn.numBits);
            recordColumn.value = value;
            break;
        }
        case NULL_TYPE: {
            // do nothing value is null
            break;
        }
        default:
            throw std::runtime_error("Invalid type ");
        }
        offset += recordColumn.contentSize;
    }
}

void SqliteFilePageReader::processAllCells() {
    for (int i = numCellsInPage - 1; i >= 0; i--) {
        buildCell(cells[i]);
    }
}


void SqliteFilePageReader::parseHeader() {
    auto pageBegin = FileOffset((pageNum - 1) * pageSize);
    if (pageNum == 1) {
        pageBegin += 100;
    }
    // Read page type
    int type = read1Byte(pageSize, pageBegin, pageCache[pageNum - 1]);
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

    //Read start first free block

    this->firstFreeBlockStart = read2Bytes(pageSize, pageBegin + 1, pageCache[pageNum - 1]);
    this->numCellsInPage = read2Bytes(pageSize, pageBegin + (3), pageCache[pageNum - 1]);
    this->cellContentAreaStart = read2Bytes(pageSize, pageBegin + (5), pageCache[pageNum - 1]);
    this->fragmentedFreeBytesInCellContentArea = read1Byte(pageSize, pageBegin + (7), pageCache[pageNum - 1]);

    if (pageType == INTERIOR_TABLE_PAGE || pageType == INTERIOR_INDEX_PAGE) {
        //        this->rightMostPointer = read4Bytes(pageBegin.operator+(8));
    }
    processCellPointers();
    if (pageNum == 1) {
        buildSqliteSchemaTable();
    }
}

std::string anyToString(const std::any& a) {
    if (!a.has_value())
        return "null";

    if (a.type() == typeid(std::string))
        return std::any_cast<std::string>(a);
    else if (a.type() == typeid(const char*))
        return std::string(std::any_cast<const char*>(a));
    else if (a.type() == typeid(char*))
        return std::string(std::any_cast<char*>(a));
    else if (a.type() == typeid(int64_t))
        return std::to_string(std::any_cast<int64_t>(a));
    else if (a.type() == typeid(double))
        return std::to_string(std::any_cast<double>(a));
    else if (a.type() == typeid(float))
        return std::to_string(std::any_cast<float>(a));
    else if (a.type() == typeid(bool))
        return std::any_cast<bool>(a) ? "true" : "false";
    else {
        // fallback: show the type name
        return std::string("<unhandled type: ") + a.type().name() + ">";
    }
}

/*
 * Sqlite Schema table is constant with the following format
 * https://www.sqlite.org/schematab.html
 * https://www.sqlite.org/fileformat.html#ffschema
 */
void SqliteFilePageReader::buildSqliteSchemaTable() {
    const char* arr[] = {"type", "name", "tbl_name", "rootpage", "sql"};
    for (int colIdx = 0; colIdx < 5; colIdx++) {
        for (auto& cell : cells) {
            cell.schema[arr[colIdx]] = cell.record.recordColumns[colIdx].content;
            if (colIdx == 3) {
                cell.schema[arr[colIdx]] = cell.record.recordColumns[colIdx].value;
            }
        }
    }
}

void SqliteFilePageReader::loadPage(int pageNum) {
    auto buffer = std::make_unique<std::uint8_t[]>(pageSize);
    dbFile->seekg((pageNum - 1) * pageSize, std::ios::beg);
    dbFile->read(reinterpret_cast<char*>(buffer.get()), pageSize);
    pageCache[(pageNum-1)] = std::move(buffer);
}

void SqliteFilePageReader::buildSchemaTableRows() {
    for (SqliteBTreeSchemaCell cell : cells) {
        SqliteSchemaTables page;
        page.type = anyToString(cell.schema["type"]);
        page.name = anyToString(cell.schema["name"]);
        if (cell.schema["rootpage"].type() == typeid(int64_t)) {
            page.rootPage = std::any_cast<int64_t>(cell.schema["rootpage"]);
        }
        page.sql = anyToString(cell.schema["sql"]);
        page.tableName = anyToString(cell.schema["tbl_name"]);
        tables.emplace_back(page);
    }
}

void SqliteFilePageReader::printTableNames() {
    for (SqliteBTreeSchemaCell cell : cells) {
        for (const auto& pair : cell.schema) {
            //            std::cout << anyToString(cell.schema["rootpage"]) << std::endl;
            if (pair.first == "type" && anyToString(pair.second) == "table" &&
                !anyToString(cell.schema["tbl_name"]).starts_with("sqlite")) {
                std::cout << anyToString(cell.schema["tbl_name"]) << " ";
            }
        }
    }
}
