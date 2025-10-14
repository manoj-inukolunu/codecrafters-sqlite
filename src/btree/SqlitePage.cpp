//
// Created by Manoj Inukolunu on 11/10/25.
//

#include "SqlitePage.h"

#include <iostream>
#include "../DebugUtils.h"

namespace btree{
    void SqlitePage::printId(Cell cell) {
        LOG_DEBUG(cell.rowId);
        std::cout << cell.rowId << std::endl;
    }

    std::vector<std::string> SqlitePage::collectColumnData(Cell cell) const {
        size_t offset = cell.cellDataOffset;
        std::vector<std::string> values;
        values.emplace_back(std::to_string(cell.rowId));
        for (int i = 0; i < cell.dataFormat.size(); i++) {
            auto format = cell.dataFormat[i];
            switch (std::get<0>(format)) {
            case TEXT: {
                LOG_DEBUG(" Text Content " << std::string(reinterpret_cast<char*>(data.get()) + offset, std::get<1>(format)));
                values.emplace_back(std::string(reinterpret_cast<char*>(data.get()) + offset, std::get<1>(format)));
                break;
            }
            case INT: {
                uint64_t value = 0;
                for (int j = 0; j < std::get<1>(format); ++j) {
                    value = (value << 8) | static_cast<uint64_t>(data[offset + j]);
                }
                LOG_DEBUG("Int Content " << value);
                values.emplace_back(std::to_string(value));
                break;
            }

            case NULL_TYPE: {
                //hack i=0 is for the rowid column
                if (i != 0) {
                    values.emplace_back("");
                }
                LOG_DEBUG("Null Type");
                break;
            }

            case BLOB: {
                throw new std::runtime_error("BLOB is not supported");
                break;
            }

            case REAL: {
                throw new std::runtime_error("REAL is not supported");
                break;
            }

            default:
                throw new std::runtime_error("Type not supported ");
            }
            offset += std::get<1>(format);
        }
        return values;
    }


    void SqlitePage::processCellPointers() {
        auto cellPointerStart = headerSize(pageType);
        if (pageNum == 1) {
            cellPointerStart += 100;
        }
        for (int i = 0; i < numCellsInPage; i++) {
            int offset = read2Bytes(pageSize, cellPointerStart + (i * 2), data);
            cellContentOffsets.push_back(offset);
            LOG_DEBUG("Cell Content Offset " << offset << " - Cell Number " << i);
        }

        if (pageType == INTERIOR_TABLE_PAGE) {
            rightMostChildPageNum = read4Bytes(pageSize, 8, data);
        }

        LOG_DEBUG("Processed Cell Pointers");
    }


    void SqlitePage::buildInteriorCell(int cellNumber) {
        Cell cell;
        cell.leftChildPageNum = read4Bytes(pageSize, cellContentOffsets[cellNumber], data);
        cell.rowId = readVarInt(pageSize, cellContentOffsets[cellNumber] + 4, data).first;
        cells.emplace_back(cell);
    }


    void SqlitePage::processAllCells() {
        for (int i = 0; i < numCellsInPage; i++) {
            LOG_DEBUG("Processing cell " << i << " at offset " << cellContentOffsets[i]);
            if (pageType == INTERIOR_TABLE_PAGE) {
                buildInteriorCell(i);
            } else if (pageType == LEAF_TABLE_PAGE) {
                buildLeafCell(i);
            }
        }
    }


    void SqlitePage::buildLeafCell(int cellNumber) {
        Cell cell;

        // 1) payload size (varint) at cell start
        auto payloadV = readVarInt(pageSize, cellContentOffsets[cellNumber], data);
        const uint64_t payloadSize = payloadV.first;
        cell.payloadSize = payloadSize;
        LOG_DEBUG("Record Size " << payloadSize);

        // 2) rowid (varint)
        auto rowIdV = readVarInt(pageSize, payloadV.second, data);
        cell.rowId = rowIdV.first;
        LOG_DEBUG(" Row Id " << cell.rowId);

        // 3) header_size (varint). Header starts at rowIdV.second.
        auto hdrV = readVarInt(pageSize, rowIdV.second, data);
        const uint64_t headerSize = hdrV.first; // bytes from header_size varint start to end of serial-types list
        size_t p = hdrV.second; // first serial-type varint
        const size_t headerStart = static_cast<size_t>(rowIdV.second);
        const size_t headerEnd = headerStart + static_cast<size_t>(headerSize);

        LOG_DEBUG(" Record Header Size " << headerSize);

        // 4) Read serial types UNTIL headerEnd (track header progress by bytes of varints, not body lengths)
        std::vector<std::tuple<DataType, long, long>> dataFormat;
        while (p < headerEnd) {
            auto stV = readVarInt(pageSize, p, data); // stV.first = serial type, stV.second = next varint pos
            const auto dt = dataType(stV.first);
            const long len = static_cast<long>(contentSize(stV.first)); // bytes in BODY for this column
            dataFormat.emplace_back(dt, len, 0L);
            LOG_DEBUG("  SerialType " << stV.first << " -> len " << len << " type " << dataTypeStr(dt));
            p = stV.second;
        }

        // 5) BODY starts exactly at headerEnd
        cell.cellDataOffset = static_cast<long>(headerEnd);

        // 6) Compute per-column BODY offsets
        long bodyOffset = static_cast<long>(cell.cellDataOffset);
        for (auto& t : dataFormat) {
            std::get<2>(t) = bodyOffset;
            bodyOffset += std::get<1>(t);
        }
        cell.dataFormat = std::move(dataFormat);

        // 7) Sanity: header_size + sum(body lengths) == payload_size
        long sumBody = 0;
        for (const auto& t : cell.dataFormat)
            sumBody += std::get<1>(t);
        const uint64_t check = static_cast<uint64_t>(headerSize) + static_cast<uint64_t>(sumBody);

        if (check != payloadSize) {
            std::ostringstream oss;
            oss << "Total Size mismatch: header(" << headerSize << ") + body(" << sumBody
                << ") = " << check << " != payload(" << payloadSize << ")";
            throw std::runtime_error(oss.str());
        }

        LOG_DEBUG("All Good - Data Offset (body start) " << cell.cellDataOffset);

        cells.emplace_back(std::move(cell));
    }


    void SqlitePage::parseHeader() {
        int pageBegin = 0;
        if (pageNum == 1) {
            pageBegin += 100;
        }
        // Read page type
        int type = read1Byte(pageSize, pageBegin, data);
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

        this->firstFreeBlockStart = read2Bytes(pageSize, pageBegin + 1, data);
        this->numCellsInPage = read2Bytes(pageSize, pageBegin + (3), data);
        this->cellContentAreaStart = read2Bytes(pageSize, pageBegin + (5), data);
        this->fragmentedFreeBytesInCellContentArea = read1Byte(pageSize, pageBegin + (7), data);

        if (pageType == INTERIOR_INDEX_PAGE) {
            throw std::runtime_error("Interior Index page is not supported");
        }
        processCellPointers();
        processAllCells();
    }
}
