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
    return value;
}

std::pair<uint64_t, FileOffset> readVarInt(FileOffset offset, std::ifstream &dbFile) {
    auto readByte = [&](FileOffset& off) -> uint8_t {
        int x = read1Byte(off, dbFile); // must return 0..255 or throw on error
        off += 1;
        return static_cast<uint8_t>(x);
    };

    uint64_t val = 0;

    // Read up to 8 continuation bytes (7 bits each). Stop if MSB==0.
    for (int i = 0; i < 8; ++i) {
        uint8_t b = readByte(offset);
        if ((b & 0x80u) == 0) {
            val = (val << 7) | b;          // last byte contributes 7 bits
            return {val, offset};
        }
        val = (val << 7) | (b & 0x7Fu);     // continuation: take low 7 bits
    }

    // If we got here, we already consumed 8 bytes with MSB=1. The 9th byte contributes 8 bits.
    uint8_t last = readByte(offset);
    val = (val << 8) | last;
    return {val, offset};
}
