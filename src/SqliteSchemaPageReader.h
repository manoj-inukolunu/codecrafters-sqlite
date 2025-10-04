//
// Created by Manoj Inukolunu on 28/09/25.
//

#ifndef SQLITE_STARTER_CPP_SQLITESCHEMAPAGEREADER_H
#define SQLITE_STARTER_CPP_SQLITESCHEMAPAGEREADER_H

#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>

#include "utils.h"
#include <cassert>
#include <map>
#include <any>


/**
 * Schema Page is the first page of the file
 */




struct SqliteBTreeSchemaCell {
    struct RecordHeader {
        varint headerSize;
        std::vector<RecordColumn> recordColumns;
    };

    int cellNumber{};
    //offset is from the beginning of the file
    FileOffset offset{};
    varint recordSize{};
    varint rowId{};
    RecordHeader record;
    std::map<std::string, std::any> schema;
    FileOffset recordBodyOffset{};
};


class SqliteSchemaPageReader {
public:
    SqliteSchemaPageReader(int pageNum, int pageSize, std::ifstream &dbFile);

    //unnecessary but just in case for clarity of the reader.
    //If pageNum == 1 then the first 100 bytes are the database dbFile header
    int pageNum;
    //unnecessary but just in case for clarity of the reader
    int pageSize;
    int pageOffset;
    BTreePageType pageType;
    int firstFreeBlockStart;
    int numCellsInPage;
    int cellContentAreaStart;
    int fragmentedFreeBytesInCellContentArea;
    //Appears only in the header of the interior b-tree page and is omitted for all other pages
    FileOffset rightMostPointer;
    std::vector<SqliteBTreeSchemaCell> cells;

    std::vector<FileOffset> cellContentOffsets;
    // The database dbFile
    std::ifstream &dbFile;

    std::pair<uint64_t, FileOffset> readVarInt(FileOffset offset);

    void processCellPointers();


    void processAllCells();

    void printTableNames();

private:

    std::vector<std::string> tableNames;
    std::vector<std::pair<FileOffset, RecordColumn>> recordDataOffsets;


    void buildCellBody(SqliteBTreeSchemaCell &cell);

    void buildCell(SqliteBTreeSchemaCell &cell);

    void buildSqliteSchemaTable();

    void parseHeader();

    int read2Bytes(FileOffset offset);

    int read1Byte(FileOffset offset);


    static uint16_t swap(uint16_t x) {
        return (x >> 8) | (x << 8);
    }

    static bool little_endian() {
        int x = 1;
        return (*reinterpret_cast<char *>(&x) == 1);
    }
};


#endif //SQLITE_STARTER_CPP_SQLITESCHEMAPAGEREADER_H
