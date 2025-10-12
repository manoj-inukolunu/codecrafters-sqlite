//
// Created by Manoj Inukolunu on 11/10/25.
//

#include "SqlitePage.h"

#include <iostream>

namespace btree{
    void SqlitePage::processCellPointers() {
        auto cellPointerStart = headerSize(pageType);
        if (pageNum == 1) {
            cellPointerStart += 100;
        }
        for (int i = 0; i < numCellsInPage; i++) {
            int offset = read2Bytes(pageSize, cellPointerStart + (i * 2), data);
            cellContentOffsets.push_back(offset);
            std::cout << "Cell Content Offset " << offset << " - Cell Number " << i << std::endl;
        }

        std::cout << "Processed Cell Pointers " << std::endl;
    }

    void SqlitePage::processAllCells() {
        for (int i = 0; i < numCellsInPage; i++) {
            std::cout << "Processing cell " << i << " at offset " << cellContentOffsets[i] << std::endl;
            buildCell(i);
        }
    }

    void SqlitePage::buildCell(int cellNumber) {
        auto sizeOfRecord = readVarInt(pageSize, cellContentOffsets[cellNumber], data);
        std::cout << "Record Size " << sizeOfRecord.first << std::endl;
        auto rowId = readVarInt(pageSize, sizeOfRecord.second, data);
        std::cout << " Row Id " << rowId.first << std::endl;
        auto recordHeaderSize = readVarInt(pageSize, rowId.second, data);
        std::cout << " Record Header Size " << recordHeaderSize.first << std::endl;

        long totalsize = recordHeaderSize.first;
        long offset = recordHeaderSize.second;
        std::vector<std::pair<DataType, long>> dataFormat;
        while (totalsize < sizeOfRecord.first) {
            std::pair<uint64_t, FileOffset> payload = readVarInt(pageSize, offset, data);
            totalsize += contentSize(payload.first);
            std::cout << "  Payload Serial Type " << payload.first << " Content Size " << contentSize(payload.first) << " dataType " << dataTypeStr(dataType(payload.first)) << std::endl;
            dataFormat.emplace_back(std::make_pair(dataType(payload.first), contentSize(payload.first)));
            if (dataType(payload.first) == INT) {
                std::cout << " Content Size in bytes =  " << static_cast<int>(contentSize(payload.first)) << std::endl;
            }
            offset = payload.second;
        }
        std::string error = std::format("Total Size {} != Size of Record {}", totalsize, sizeOfRecord.first).c_str();
        if (totalsize != sizeOfRecord.first) {
            throw std::runtime_error(error);
        }
        std::cout << "All Good " << " Data Offset " << offset << std::endl;

        for (auto format : dataFormat) {
            switch (format.first) {
            case TEXT: {
                std::cout << " Text Content " << std::string(reinterpret_cast<char*>(data.get()) + offset, format.second) << std::endl;
                break;
            }
            case INT: {
                uint64_t value = 0;
                for (int i = 0; i < format.second; ++i) {
                    value = (value << 8) | static_cast<uint64_t>(data[offset + i]);
                }
                std::cout << "Int Content " << value << std::endl;
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
            offset += format.second;
        }
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
