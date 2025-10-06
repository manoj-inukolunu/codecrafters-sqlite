#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include <variant>
#include <stdexcept>
#include <iomanip>

// ---------- Helpers: Big-endian reads ----------
static uint16_t be16(const uint8_t* p) {
    return (static_cast<uint16_t>(p[0]) << 8) | p[1];
}
static uint32_t be32(const uint8_t* p) {
    return (static_cast<uint32_t>(p[0]) << 24) |
           (static_cast<uint32_t>(p[1]) << 16) |
           (static_cast<uint32_t>(p[2]) << 8)  |
           static_cast<uint32_t>(p[3]);
}
static uint64_t beN_to_u64(const uint8_t* p, size_t n) {
    uint64_t v = 0;
    for (size_t i = 0; i < n; ++i) v = (v << 8) | p[i];
    return v;
}
static int64_t beN_to_i64(const uint8_t* p, size_t n) {
    // Read as unsigned, then sign-extend
    uint64_t u = beN_to_u64(p, n);
    int shift = 64 - static_cast<int>(n * 8);
    return (int64_t)((u << shift) >> shift); // arithmetic shift after cast
}

// ---------- SQLite varint (1..9 bytes) ----------
struct Varint {
    uint64_t value;
    size_t   bytes;  // how many bytes were consumed
};

// Decodes a SQLite varint at buf[pos]. pos will not be advanced here.
static Varint readVarint(const uint8_t* buf, size_t len, size_t pos) {
    // Follows https://www.sqlite.org/fileformat2.html#varint
    // Returns {value, bytesConsumed}
    if (pos >= len) throw std::runtime_error("varint out of bounds");

    uint64_t val = 0;
    size_t i = 0;
    for (; i < 9 && pos + i < len; ++i) {
        uint8_t c = buf[pos + i];
        if (i == 8) { // 9th byte: use all 8 bits
            val = (val << 8) | c;
            ++i;
            break;
        }
        val = (val << 7) | (c & 0x7F);
        if ((c & 0x80) == 0) { // msb 0 -> last byte
            ++i;
            break;
        }
    }
    if (i == 0) throw std::runtime_error("invalid varint");
    return {val, i};
}

// ---------- Page IO ----------
std::vector<uint8_t> readPage(std::ifstream& f, uint32_t pageNo, size_t pageSize) {
    if (pageNo < 1) throw std::runtime_error("pageNo starts at 1");
    std::vector<uint8_t> buf(pageSize);
    std::streampos off = static_cast<std::streampos>((pageNo - 1) * pageSize);
    f.seekg(off, std::ios::beg);
    f.read(reinterpret_cast<char*>(buf.data()), pageSize);
    if (f.gcount() != static_cast<std::streamsize>(pageSize)) {
        throw std::runtime_error("Incomplete page read");
    }
    return buf;
}

// ---------- B-tree header ----------
struct BtreeHeader {
    uint8_t  pageType;
    uint16_t firstFreeblock;
    uint16_t numCells;
    uint16_t cellContentStart;
    uint8_t  fragmentedFreeBytes;
    bool     isInterior;
    uint32_t rightMostPtr;
    uint16_t headerSize; // 8 (leaf) or 12 (interior)
};

BtreeHeader parseBtreeHeader(const std::vector<uint8_t>& page, bool isPage1) {
    size_t hOff = isPage1 ? 100 : 0;
    const uint8_t* p = page.data() + hOff;

    BtreeHeader h;
    h.pageType            = p[0];
    h.firstFreeblock      = be16(p + 1);
    h.numCells            = be16(p + 3);
    h.cellContentStart    = be16(p + 5);
    h.fragmentedFreeBytes = p[7];
    h.isInterior = (h.pageType == 0x02 || h.pageType == 0x05);
    h.headerSize = h.isInterior ? 12 : 8;
    h.rightMostPtr = h.isInterior ? be32(p + 8) : 0;

    return h;
}

// ---------- Record decoding (header + body) ----------
enum class ColType { Null, Int, Float, Text, Blob };

struct ColumnValue {
    ColType type = ColType::Null;
    std::variant<std::monostate, int64_t, double, std::string, std::vector<uint8_t>> data;
};

struct Record {
    std::vector<ColumnValue> cols;
};

// Serial type → (#bytes in body, ColType, special cases)
static ColumnValue decodeValue(const uint8_t* body, size_t bodyLen, size_t& pos, uint64_t serialType) {
    ColumnValue out;
    if (serialType == 0) {
        out.type = ColType::Null;
        return out;
    }
    if (serialType == 1) { // 1-byte signed int
        if (pos + 1 > bodyLen) throw std::runtime_error("int1 OOB");
        out.type = ColType::Int;
        out.data = (int64_t)beN_to_i64(body + pos, 1);
        pos += 1;
        return out;
    }
    if (serialType == 2) { // 2-byte signed int
        if (pos + 2 > bodyLen) throw std::runtime_error("int2 OOB");
        out.type = ColType::Int;
        out.data = (int64_t)beN_to_i64(body + pos, 2);
        pos += 2;
        return out;
    }
    if (serialType == 3) { // 3-byte signed int
        if (pos + 3 > bodyLen) throw std::runtime_error("int3 OOB");
        out.type = ColType::Int;
        out.data = (int64_t)beN_to_i64(body + pos, 3);
        pos += 3;
        return out;
    }
    if (serialType == 4) { // 4-byte signed int
        if (pos + 4 > bodyLen) throw std::runtime_error("int4 OOB");
        out.type = ColType::Int;
        out.data = (int64_t)beN_to_i64(body + pos, 4);
        pos += 4;
        return out;
    }
    if (serialType == 5) { // 6-byte signed int
        if (pos + 6 > bodyLen) throw std::runtime_error("int6 OOB");
        out.type = ColType::Int;
        out.data = (int64_t)beN_to_i64(body + pos, 6);
        pos += 6;
        return out;
    }
    if (serialType == 6) { // 8-byte signed int
        if (pos + 8 > bodyLen) throw std::runtime_error("int8 OOB");
        out.type = ColType::Int;
        out.data = (int64_t)beN_to_i64(body + pos, 8);
        pos += 8;
        return out;
    }
    if (serialType == 7) { // 8-byte float
        if (pos + 8 > bodyLen) throw std::runtime_error("float8 OOB");
        uint64_t u = beN_to_u64(body + pos, 8);
        double d;
        static_assert(sizeof(double) == 8, "double not 8 bytes?");
        std::memcpy(&d, &u, 8); // bitwise copy
        out.type = ColType::Float;
        out.data = d;
        pos += 8;
        return out;
    }
    if (serialType == 8) { // integer 0 (no bytes)
        out.type = ColType::Int;
        out.data = (int64_t)0;
        return out;
    }
    if (serialType == 9) { // integer 1 (no bytes)
        out.type = ColType::Int;
        out.data = (int64_t)1;
        return out;
    }
    if (serialType == 10 || serialType == 11) {
        // Reserved for internal use; shouldn't appear in table content
        throw std::runtime_error("reserved serial type 10/11 encountered");
    }

    // 12+ : blobs/text
    if (serialType >= 12) {
        bool isText = (serialType % 2 == 1);
        uint64_t nBytes = isText ? (serialType - 13) / 2 : (serialType - 12) / 2;
        if (pos + nBytes > bodyLen) throw std::runtime_error("blob/text OOB");
        if (isText) {
            out.type = ColType::Text;
            out.data = std::string(reinterpret_cast<const char*>(body + pos),
                                   reinterpret_cast<const char*>(body + pos + nBytes));
        } else {
            out.type = ColType::Blob;
            out.data = std::vector<uint8_t>(body + pos, body + pos + nBytes);
        }
        pos += static_cast<size_t>(nBytes);
        return out;
    }

    throw std::runtime_error("unknown serial type");
}

static Record parseRecord(const uint8_t* payload, size_t payloadLen) {
    // Record header: varint headerSize, then sequence of varint serial types
    size_t pos = 0;
    Varint hsz = readVarint(payload, payloadLen, pos);
    if (hsz.bytes > payloadLen) throw std::runtime_error("header varint OOB");
    size_t headerEnd = pos + hsz.bytes; // careful: we haven’t advanced pos yet

    // Advance past headerSize varint itself
    pos += hsz.bytes;

    // Read serial types until we reach headerEnd
    std::vector<uint64_t> serials;
    while (pos < headerEnd) {
        Varint st = readVarint(payload, payloadLen, pos);
        serials.push_back(st.value);
        pos += st.bytes;
    }
    if (pos != headerEnd) throw std::runtime_error("record header size mismatch");

    // Body starts at headerEnd
    size_t bodyPos = headerEnd;
    Record r;
    r.cols.reserve(serials.size());
    for (uint64_t st : serials) {
        ColumnValue cv = decodeValue(payload, payloadLen, bodyPos, st);
        r.cols.push_back(std::move(cv));
    }
    if (bodyPos > payloadLen) throw std::runtime_error("record body overflow");
    return r;
}

// ---------- Page 2 table-leaf cell parsing ----------
int __main() {
    const char* filename = "/Users/minukolunu/Projects/code-crafters/codecrafters-sqlite-cpp/sample.db";
    const size_t pageSize = 4096;
    const uint32_t pageNo = 2;

    std::ifstream f(filename, std::ios::binary);
    if (!f) {
        std::cerr << "Failed to open file\n";
        return 1;
    }

    auto page = readPage(f, pageNo, pageSize);
    BtreeHeader h = parseBtreeHeader(page, /*isPage1=*/false);

    if (h.pageType != 0x0D) {
        std::cerr << "Page " << pageNo << " is not a table leaf (0x0D). Got 0x"
                  << std::hex << (int)h.pageType << std::dec << "\n";
        return 1;
    }

    // Cell pointer array begins right after header
    size_t headerOff = 0; // page 2: header at offset 0
    size_t cellPtrBase = headerOff + h.headerSize;

    std::cout << "Page " << pageNo << " (table leaf) with " << h.numCells << " cells\n";

    // Iterate each cell
    for (uint16_t i = 0; i < h.numCells; ++i) {
        uint16_t cellOff = be16(page.data() + cellPtrBase + i * 2);

        // Parse cell at page[cellOff..]
        size_t pos = cellOff;

        std::cout << pos << std::endl;

        // 1) varint payloadSize
        Varint payloadV = readVarint(page.data(), page.size(), pos);
        std::cout << "PayloadV " << payloadV.value << std::endl;

        uint64_t payloadSize = payloadV.value;
        pos += payloadV.bytes;

        // 2) varint rowid
        Varint rowidV = readVarint(page.data(), page.size(), pos);
        uint64_t rowid = rowidV.value;
        pos += rowidV.bytes;

        // 3) payload (record)
        if (pos + payloadSize > page.size()) {
            std::cerr << "Cell " << i << ": payload spills beyond page or needs overflow handling. Skipping.\n";
            continue; // For now, skip big rows (overflow not implemented in this snippet)
        }
        const uint8_t* payload = page.data() + pos;
        size_t payloadLen = static_cast<size_t>(payloadSize);

        Record rec;
        try {
            rec = parseRecord(payload, payloadLen);
        } catch (const std::exception& e) {
            std::cerr << "Cell " << i << ": record parse error: " << e.what() << "\n";
            continue;
        }

        // Print a readable row
        std::cout << "— Cell " << i << " @ " << cellOff << " — rowid=" << rowid
                  << " payloadSize=" << payloadSize << "\n";
        for (size_t c = 0; c < rec.cols.size(); ++c) {
            const auto& cv = rec.cols[c];
            std::cout << "   col[" << c << "] = ";
            switch (cv.type) {
                case ColType::Null:
                    std::cout << "NULL";
                    break;
                case ColType::Int:
                    std::cout << std::get<int64_t>(cv.data);
                    break;
                case ColType::Float:
                    std::cout << std::get<double>(cv.data);
                    break;
                case ColType::Text: {
                    const auto& s = std::get<std::string>(cv.data);
                    // Limit long prints
                    if (s.size() > 80) std::cout << "\"" << s.substr(0, 77) << "...\"";
                    else std::cout << "\"" << s << "\"";
                    break;
                }
                case ColType::Blob: {
                    const auto& b = std::get<std::vector<uint8_t>>(cv.data);
                    std::cout << "x'" << std::hex << std::setfill('0');
                    size_t show = std::min<size_t>(b.size(), 24);
                    for (size_t k = 0; k < show; ++k) std::cout << std::setw(2) << (int)b[k];
                    if (b.size() > show) std::cout << "...";
                    std::cout << "'" << std::dec;
                    break;
                }
            }
            std::cout << "\n";
        }
    }

    return 0;
}
