//
// Created by Manoj Inukolunu on 04/10/25.
//

#include "SqliteUtils.h"

#include <cstdint>


uint16_t swap(uint16_t x) {
    return (x >> 8) | (x << 8);
}

bool little_endian() {
    int x = 1;
    return (*reinterpret_cast<char *>(&x) == 1);
}

int read2Bytes(FileOffset offset, std::ifstream &file) {
    FileOffset current = file.tellg();
    file.seekg(offset, std::ios::beg);
    size_t n = 2;
    uint16_t value = 0;
    file.read(reinterpret_cast<char *>(&value), n);
    if (little_endian()) {
        value = swap(value);
    }
    file.seekg(current, std::ios::beg);
    return value;
}


int read1Byte(FileOffset offset, std::ifstream &dbFile) {
    FileOffset current = dbFile.tellg();
    dbFile.seekg(offset, std::ios::beg);
    size_t n = 1;
    uint16_t value = 0;
    dbFile.read(reinterpret_cast<char *>(&value), n);
    dbFile.seekg(current, std::ios::beg);
    return value;
}

std::pair<uint64_t, FileOffset> readVarInt(FileOffset offset, std::ifstream &dbFile) {
    auto isMsbZero = [](int val) {
        return ((val >> 7) & 0x01) == 0;
    };
    uint64_t accum = 0;
    std::uint8_t data[9];
    int breakPoint = 0;
    for (int i = 0; i < 9; i++) {
        int currentByte = read1Byte(offset, dbFile);
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