//
// Created by Manoj Inukolunu on 04/10/25.
//

#include "SqliteUtils.h"

#include <cstdint>
#include <memory>


uint16_t swap(uint16_t x) {
    return (x >> 8) | (x << 8);
}

bool little_endian() {
    int x = 1;
    return (*reinterpret_cast<char*>(&x) == 1);
}

int read2Bytes(std::size_t pageSize, std::size_t offset, std::unique_ptr<std::uint8_t[]>& page) {
    if (!page) {
        throw std::invalid_argument("read2BytesBE: page pointer is null");
    }
    if (offset + 2 > pageSize) {
        throw std::out_of_range("read2BytesBE: offset out of bounds");
    }
    const std::uint8_t* data = page.get();
    return (static_cast<uint16_t>(data[offset]) << 8) |
        static_cast<uint16_t>(data[offset + 1]);
}


int read1Byte(std::size_t pageSize, std::size_t offset, std::unique_ptr<std::uint8_t[]>& page) {
    if (!page || offset >= pageSize)
        throw std::out_of_range("read1Byte: offset out of bounds");
    return page[offset];
}


std::pair<uint64_t, FileOffset> readVarInt(std::size_t pageSize, std::size_t offset, std::unique_ptr<std::uint8_t[]>& page) {
    auto readByte = [&](std::size_t& off) -> uint8_t {
        int x = read1Byte(pageSize,off, page); // must return 0..255 or throw on error
        off += 1;
        return static_cast<uint8_t>(x);
    };

    uint64_t val = 0;

    // Read up to 8 continuation bytes (7 bits each). Stop if MSB==0.
    for (int i = 0; i < 8; ++i) {
        uint8_t b = readByte(offset);
        if ((b & 0x80u) == 0) {
            val = (val << 7) | b; // last byte contributes 7 bits
            return {val, offset};
        }
        val = (val << 7) | (b & 0x7Fu); // continuation: take low 7 bits
    }

    // If we got here, we already consumed 8 bytes with MSB=1. The 9th byte contributes 8 bits.
    uint8_t last = readByte(offset);
    val = (val << 8) | last;
    return {val, offset};
}
