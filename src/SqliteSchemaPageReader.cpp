//
// Created by Manoj Inukolunu on 28/09/25.
//

#include "SqliteSchemaPageReader.h"

#include <filesystem>
#include <format>
#include <vector>


SqliteSchemaPageReader::SqliteSchemaPageReader(int pageNum, int pageSize, std::ifstream &dbFile) : dbFile(dbFile),
                                                                                                   pageNum(pageNum),
                                                                                                   pageSize(pageSize) {
    if (pageNum == 0) {
        throw std::runtime_error("Invalid Page Number 0");
    }
    if (pageNum == 1) {
        this->parseHeader();
    } else {
        this->parseHeader();
    }
}


void SqliteSchemaPageReader::processCellPointers() {
    int pageHeaderSize = (this->pageType == LEAF_TABLE_PAGE || this->pageType == LEAF_INDEX_PAGE) ? 8 : 12;

    auto cellPointerStart = std::ifstream::pos_type((pageNum - 1) * pageSize).operator+(pageHeaderSize);
    if (pageNum == 1) {
        cellPointerStart += 100;
    }
    for (int i = 0; i < this->numCellsInPage; i++) {
        SqliteBTreeSchemaCell cell;
        cell.offset = read2Bytes(cellPointerStart, dbFile);
        cell.cellNumber = i;
        cellContentOffsets.emplace_back(cell.offset);
        cellPointerStart += 2;
        cells.emplace_back(cell);
    }
    processAllCells();
}

void SqliteSchemaPageReader::buildCell(SqliteBTreeSchemaCell &cell) {
    auto sizeOfRecord = readVarInt(cellContentOffsets[cell.cellNumber], dbFile);
    cell.recordSize = sizeOfRecord.first;
    auto rowId = readVarInt(sizeOfRecord.second, dbFile);
    cell.rowId = rowId.first;
    SqliteBTreeSchemaCell::RecordHeader recordHeader;
    auto recordHeaderSize = readVarInt(rowId.second, dbFile);
    recordHeader.headerSize = recordHeaderSize.first;

    FileOffset columnOffSet = recordHeaderSize.second;
    std::vector<RecordColumn> columns;
    long totalsize = recordHeader.headerSize;
    while (totalsize < cell.recordSize) {
        std::pair<uint64_t, FileOffset> payload = readVarInt(columnOffSet, dbFile);
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

void SqliteSchemaPageReader::buildCellBody(SqliteBTreeSchemaCell &cell) {
    FileOffset offset = cell.recordBodyOffset;
    for (auto &recordColumn: cell.record.recordColumns) {
        switch (recordColumn.type) {
            case TEXT: {
                std::unique_ptr<char[]> buffer(new char[recordColumn.contentSize]());
                dbFile.seekg(offset, std::ios::beg);
                dbFile.read(buffer.get(), recordColumn.contentSize);
                // Construct string from raw bytes
                std::string result(buffer.get(), recordColumn.contentSize);
                recordColumn.content.assign(buffer.get(), recordColumn.contentSize);
                break;
            }
            case INT: {
                dbFile.seekg(offset, std::ios::beg);
                int64_t value = 0;
                dbFile.read(reinterpret_cast<char *>(&value), recordColumn.numBits);
                recordColumn.value = value;
                break;
            }
            default:
                throw std::runtime_error("Invalid type ");
        }
        offset += recordColumn.contentSize;
    }
}

void SqliteSchemaPageReader::processAllCells() {
    for (int i = numCellsInPage - 1; i >= 0; i--) {
        buildCell(cells[i]);
    }
}


void SqliteSchemaPageReader::parseHeader() {
    auto pageBegin = FileOffset((pageNum - 1) * pageSize);
    if (pageNum == 1) {
        pageBegin += 100;
    }
    // Read page type
    int type = read1Byte(pageBegin, dbFile);
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

    this->firstFreeBlockStart = read2Bytes(pageBegin + 1, dbFile);
    this->numCellsInPage = read2Bytes(pageBegin + (3), dbFile);
    this->cellContentAreaStart = read2Bytes(pageBegin + (5), dbFile);
    this->fragmentedFreeBytesInCellContentArea = read1Byte(pageBegin + (7), dbFile);

    if (pageType == INTERIOR_TABLE_PAGE || pageType == INTERIOR_INDEX_PAGE) {
        //        this->rightMostPointer = read4Bytes(pageBegin.operator+(8));
    }


    processCellPointers();
    buildSqliteSchemaTable();
}

std::string anyToString(const std::any &a) {
    if (!a.has_value()) return "null";

    if (a.type() == typeid(std::string))
        return std::any_cast<std::string>(a);
    else if (a.type() == typeid(const char *))
        return std::string(std::any_cast<const char *>(a));
    else if (a.type() == typeid(char *))
        return std::string(std::any_cast<char *>(a));
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
void SqliteSchemaPageReader::buildSqliteSchemaTable() {
    const char *arr[] = {"type", "name", "tbl_name", "rootpage", "sql"};
    for (int colIdx = 0; colIdx < 5; colIdx++) {
        for (auto &cell: cells) {
            cell.schema[arr[colIdx]] = cell.record.recordColumns[colIdx].content;
            if (colIdx == 3) {
                cell.schema[arr[colIdx]] = cell.record.recordColumns[colIdx].value;
            }
        }
    }
}

void SqliteSchemaPageReader::buildSchemaTableRows() {
    for (SqliteBTreeSchemaCell cell: cells) {
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

    for (auto table: tables) {
        table.toString();
    }
}

void SqliteSchemaPageReader::printTableNames() {
    for (SqliteBTreeSchemaCell cell: cells) {
        for (const auto &pair: cell.schema) {
//            std::cout << anyToString(cell.schema["rootpage"]) << std::endl;
            if (pair.first == "type" && anyToString(pair.second) == "table" &&
                !anyToString(cell.schema["tbl_name"]).starts_with("sqlite")) {
                std::cout << anyToString(cell.schema["tbl_name"]) << " " << anyToString(cell.schema["rootpage"]) << " ";
            }
        }
    }

}


