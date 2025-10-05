//
// Created by Manoj Inukolunu on 04/10/25.
//

#ifndef SQLITE_STARTER_CPP_SQLITEUTILS_H
#define SQLITE_STARTER_CPP_SQLITEUTILS_H


#include <cstdint>
#include <iostream>
#include <fstream>


typedef long varint;
typedef std::streamoff FileOffset;

enum BTreePageType {
    INTERIOR_INDEX_PAGE,
    INTERIOR_TABLE_PAGE,
    LEAF_INDEX_PAGE,
    LEAF_TABLE_PAGE
};

enum DataType {
    INT,
    REAL,
    TEXT,
    BLOB,
    NULL_TYPE
};

inline const char *dataTypeStr(DataType type) {
    switch (type) {
        case INT:
            return "INT";
        case REAL:
            return "REAL";
        case TEXT:
            return "TEXT";
        case BLOB:
            return "BLOB";
        case NULL_TYPE:
            return "NULL_TYPE";
    }
    throw new std::invalid_argument("Invalid data type");
}

struct RecordColumn {
    varint serialType;
    long contentSize;
    DataType type;
    std::uint8_t numBits;
    std::string content;
    int64_t value;
};


inline long contentSize(long data) {
    if (data == 11 || data == 10) {
        throw std::runtime_error("This type is not supported in a live database file");
    }
    if (data >= 12 && data % 2 == 0) {
        return (data - 12) / 2;
    }
    if (data >= 13 && data % 2 != 0) {
        return (data - 13) / 2;
    }
    int typeArray[10] = {0, 1, 2, 3, 4, 6, 8, 0, 0};
    return typeArray[data];
}

inline DataType dataType(varint serialType) {
    if (serialType == 7) {
        return REAL;
    }
    if (serialType == 0) {
        return NULL_TYPE;
    }
    if (serialType <= 11) {
        return INT;
    }
    if (serialType >= 12 && serialType % 2 == 0) {
        return BLOB;
    }
    if (serialType >= 13 && serialType % 2 != 0) {
        return TEXT;
    }
    throw std::runtime_error("This type is not supported in a live database file");
}

inline uint8_t numBits(varint serialType) {
    int array[] = {0, 1, 2, 3, 4, 6, 8, 8, 0, 0, -1, -1, -1, -1};
    return array[serialType];
}

int read2Bytes(FileOffset offset, std::ifstream &dbFile);

int read1Byte(FileOffset offset, std::ifstream &dbFile);

uint16_t swap(uint16_t x);

bool little_endian();

std::pair<uint64_t, FileOffset> readVarInt(FileOffset offset, std::ifstream &dbFile);

#endif //SQLITE_STARTER_CPP_SQLITEUTILS_H
