//
// Created by Manoj Inukolunu on 28/09/25.
//

#ifndef SQLITE_STARTER_CPP_SQLITESCHEMAPAGEREADER_H
#define SQLITE_STARTER_CPP_SQLITESCHEMAPAGEREADER_H

#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>

enum BTreePageType {
    INTERIOR_INDEX_PAGE,
    INTERIOR_TABLE_PAGE,
    LEAF_INDEX_PAGE,
    LEAF_TABLE_PAGE
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
    long rightMostPointer;

    std::vector<long> cellContentOffsets;
    // The database dbFile
    std::ifstream &dbFile;

    std::pair<uint64_t, uint64_t> readVarInt(std::fpos<mbstate_t> offset);

    void printCellPointers();

    void readAndPrintCell(int cellNumber);

    void printRecordBody();


private:

    std::vector<std::pair<long,long>> recordDataOffsets;

    void parseHeader();

    int read2Bytes(std::ifstream::pos_type offset);

    int read1Byte(std::ifstream::pos_type offset);

    int read4Bytes(std::ifstream::pos_type offset);

    static uint16_t swap(uint16_t x) {
        return (x >> 8) | (x << 8);
    }

    static bool little_endian() {
        int x = 1;
        return (*reinterpret_cast<char *>(&x) == 1);
    }


};


#endif //SQLITE_STARTER_CPP_SQLITESCHEMAPAGEREADER_H
