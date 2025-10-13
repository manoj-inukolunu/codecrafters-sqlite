//
// Created by Manoj Inukolunu on 11/10/25.
//

#include "SqlitePage.h"

#include <iostream>

namespace btree{
    void SqlitePage::printId(Cell cell) {
        std::cerr << cell.rowId << std::endl;
    }

    void SqlitePage::printColumn(Cell cell, int columnIndex) const {
        size_t offset = cell.cellDataOffset;
        for (int i = 0; i < cell.dataFormat.size(); i++) {
            auto format = cell.dataFormat[i];
            if (i != columnIndex) {
                offset += std::get<1>(format);
                continue;
            }
            switch (std::get<0>(format)) {
            case TEXT: {
                std::cerr << " Text Content " << std::string(reinterpret_cast<char*>(data.get()) + offset, std::get<1>(format)) << std::endl;
                break;
            }
            case INT: {
                uint64_t value = 0;
                for (int j = 0; j < std::get<1>(format); ++j) {
                    value = (value << 8) | static_cast<uint64_t>(data[offset + j]);
                }
                std::cerr << "Int Content " << value << std::endl;
                break;
            }

            case NULL_TYPE: {
                std::cerr << "Null Type" << std::endl;
                break;
            }

            case BLOB: {
                break;
            }

            case REAL: {
                break;
            }

            default:
                throw new std::runtime_error("Type not supported ");
            }
            offset += std::get<1>(format);
        }
    }


    void SqlitePage::printAllCellData(Cell cell) {
        std::cerr << "Cell Row Id " << cell.rowId << " Payload Size " << cell.payloadSize << std::endl;
        size_t offset = cell.cellDataOffset;
        for (auto format : cell.dataFormat) {
            switch (std::get<0>(format)) {
            case TEXT: {
                std::cerr << " Text Content " << std::string(reinterpret_cast<char*>(data.get()) + offset, std::get<1>(format)) << std::endl;
                break;
            }
            case INT: {
                uint64_t value = 0;
                for (int i = 0; i < std::get<1>(format); ++i) {
                    value = (value << 8) | static_cast<uint64_t>(data[offset + i]);
                }
                std::cerr << "Int Content " << value << std::endl;
                break;
            }

            case NULL_TYPE: {
                break;
            }

            case BLOB: {
                break;
            }

            case REAL: {
                break;
            }

            default:
                throw new std::runtime_error("Type not supported ");
            }
            offset += std::get<1>(format);
        }
    }


    void SqlitePage::processCellPointers() {
        auto cellPointerStart = headerSize(pageType);
        if (pageNum == 1) {
            cellPointerStart += 100;
        }
        for (int i = 0; i < numCellsInPage; i++) {
            int offset = read2Bytes(pageSize, cellPointerStart + (i * 2), data);
            cellContentOffsets.push_back(offset);
            std::cerr << "Cell Content Offset " << offset << " - Cell Number " << i << std::endl;
        }

        std::cerr << "Processed Cell Pointers " << std::endl;
    }

    void SqlitePage::processAllCells() {
        for (int i = 0; i < numCellsInPage; i++) {
            std::cerr << "Processing cell " << i << " at offset " << cellContentOffsets[i] << std::endl;
            buildCell(i);
        }
    }


    void SqlitePage::buildCell(int cellNumber) {
        Cell cell;
        auto sizeOfRecord = readVarInt(pageSize, cellContentOffsets[cellNumber], data);
        cell.payloadSize = sizeOfRecord.first;

        std::cerr << "Record Size " << sizeOfRecord.first << std::endl;
        auto rowId = readVarInt(pageSize, sizeOfRecord.second, data);
        std::cerr << " Row Id " << rowId.first << std::endl;
        cell.rowId = rowId.first;
        auto recordHeaderSize = readVarInt(pageSize, rowId.second, data);
        std::cerr << " Record Header Size " << recordHeaderSize.first << std::endl;

        long totalsize = recordHeaderSize.first;
        long offset = recordHeaderSize.second;
        std::vector<std::tuple<DataType, long, long>> dataFormat;

        while (totalsize < sizeOfRecord.first) {
            std::pair<uint64_t, FileOffset> payload = readVarInt(pageSize, offset, data);
            totalsize += contentSize(payload.first);
            std::cerr << "  Payload Serial Type " << payload.first << " Content Size " << contentSize(payload.first) << " dataType " << dataTypeStr(dataType(payload.first)) << std::endl;
            auto tuple = std::make_tuple(dataType(payload.first), contentSize(payload.first), offset);
            dataFormat.emplace_back(tuple);
            if (dataType(payload.first) == INT) {
                std::cerr << " Content Size in bytes =  " << static_cast<int>(contentSize(payload.first)) << std::endl;
            }
            offset = payload.second;
        }
        cell.dataFormat = dataFormat;
        cell.cellDataOffset = offset;
        std::string error = std::format("Total Size {} != Size of Record {}", totalsize, sizeOfRecord.first).c_str();
        //set the offsets for content
        std::get<2>(cell.dataFormat[0]) = offset;
        for (int i = 1; i < cell.dataFormat.size(); i++) {
            std::get<2>(cell.dataFormat[i]) = std::get<2>(cell.dataFormat[i - 1]) + std::get<1>(cell.dataFormat[i - 1]);
        }
        if (totalsize != sizeOfRecord.first) {
            throw std::runtime_error(error);
        }
        std::cerr << "All Good " << " Data Offset " << offset << std::endl;
        cells.emplace_back(cell);
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

        if (pageType == INTERIOR_TABLE_PAGE || pageType == INTERIOR_INDEX_PAGE) {
            throw std::runtime_error(std::format("Interior Table page not supported"));
        }
        processCellPointers();
        processAllCells();
    }
}
