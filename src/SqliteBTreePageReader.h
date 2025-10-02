//
// Created by Manoj Inukolunu on 28/09/25.
//

#ifndef SQLITE_STARTER_CPP_SQLITEBTREEPAGEREADER_H
#define SQLITE_STARTER_CPP_SQLITEBTREEPAGEREADER_H

#include <iostream>
#include <fstream>
#include <cstdint>

enum BTreePageType {
    INTERIOR_INDEX_PAGE,
    INTERIOR_TABLE_PAGE,
    LEAF_INDEX_PAGE,
    LEAF_TABLE_PAGE
};

class SqliteBTreePageReader {

public:

    SqliteBTreePageReader(int pageNum, int pageSize, std::ifstream &dbFile);

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

    // The database dbFile
    std::ifstream &dbFile;

    uint64_t readVarInt(std::fpos<mbstate_t> offset);

    void printCellPointers();


private:
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


#endif //SQLITE_STARTER_CPP_SQLITEBTREEPAGEREADER_H
