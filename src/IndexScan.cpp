//
// Created by Manoj Inukolunu on 16/10/25.
//

#include "IndexScan.h"
#include "btree/SqlitePage.h"
#include "DebugUtils.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

// Helper function to load a page from file
static std::unique_ptr<btree::SqlitePage> loadIndexPage(const std::string& dbPath, int pageNum, int pageSize) {
    std::ifstream stream(dbPath, std::ios::binary);
    if (!stream.is_open()) {
        throw std::runtime_error("Failed to open database file");
    }

    stream.seekg((pageNum - 1) * pageSize, std::ios::beg);
    auto buffer = std::make_unique<uint8_t[]>(pageSize);
    stream.read(reinterpret_cast<char*>(buffer.get()), pageSize);

    return std::make_unique<btree::SqlitePage>(pageSize, pageNum, std::move(buffer));
}

// Helper function to extract string value from cell data
static std::string extractStringValue(const btree::Cell& cell, const std::unique_ptr<uint8_t[]>& pageData, int columnIndex) {
    if (columnIndex >= cell.dataFormat.size()) {
        throw std::runtime_error("Column index out of range");
    }

    const auto& format = cell.dataFormat[columnIndex];
    DataType type = std::get<0>(format);
    long length = std::get<1>(format);
    long offset = std::get<2>(format);

    if (type == TEXT) {
        return std::string(reinterpret_cast<char*>(pageData.get()) + offset, length);
    } else if (type == INT) {
        uint64_t value = 0;
        for (int i = 0; i < length; ++i) {
            value = (value << 8) | static_cast<uint64_t>(pageData[offset + i]);
        }
        return std::to_string(value);
    } else if (type == NULL_TYPE) {
        return "";
    }

    throw std::runtime_error("Unsupported data type in index");
}

// Helper function to extract rowId (last column in index entry)
static long extractRowId(const btree::Cell& cell, const std::unique_ptr<uint8_t[]>& pageData) {
    if (cell.dataFormat.empty()) {
        throw std::runtime_error("Empty dataFormat in index cell");
    }

    // In index B-trees, the last column is the rowId
    const auto& format = cell.dataFormat[cell.dataFormat.size() - 1];
    DataType type = std::get<0>(format);
    long length = std::get<1>(format);
    long offset = std::get<2>(format);

    if (type != INT) {
        throw std::runtime_error("RowId must be integer type");
    }

    uint64_t value = 0;
    for (int i = 0; i < length; ++i) {
        value = (value << 8) | static_cast<uint64_t>(pageData[offset + i]);
    }

    return static_cast<long>(value);
}

// Helper function to recursively traverse and collect matching rowIds
static void traverseAndCollect(const std::string& dbPath, int pageNum, int pageSize,
                                const std::string& keyToSearchFor, std::vector<long>& results) {
    auto page = loadIndexPage(dbPath, pageNum, pageSize);

    LOG_DEBUG("=== Traversing page " << pageNum << " ===");
    LOG_DEBUG("  Page type: " << (page->pageType == LEAF_INDEX_PAGE ? "LEAF_INDEX" :
                                   page->pageType == INTERIOR_INDEX_PAGE ? "INTERIOR_INDEX" : "OTHER"));
    LOG_DEBUG("  Number of cells (header): " << page->numCellsInPage);
    LOG_DEBUG("  Number of cells (vector): " << page->cells.size());

    if (page->numCellsInPage != page->cells.size()) {
        LOG_DEBUG("  WARNING: Cell count mismatch! Header says " << page->numCellsInPage << " but vector has " << page->cells.size());
    }

    if (page->pageType == LEAF_INDEX_PAGE) {
        // Search through leaf cells for ALL matching keys
        LOG_DEBUG("  Searching " << page->cells.size() << " leaf cells");
        for (size_t i = 0; i < page->cells.size(); i++) {
            const auto& cell = page->cells[i];
            std::string cellKey = extractStringValue(cell, page->data, 0);

            LOG_DEBUG("  Cell[" << i << "] key: '" << cellKey << "' (dataFormat size: " << cell.dataFormat.size() << ")");

            if (cellKey == keyToSearchFor) {
                long rowId = extractRowId(cell, page->data);
                LOG_DEBUG("  *** MATCH FOUND *** Cell[" << i << "] key='" << cellKey << "' rowId=" << rowId);
                results.push_back(rowId);
            }
        }
        LOG_DEBUG("  Finished leaf page " << pageNum << ", found " << results.size() << " total matches so far");
    } else if (page->pageType == INTERIOR_INDEX_PAGE) {
        LOG_DEBUG("  Interior page with " << page->cells.size() << " cells, rightmost child: " << page->rightMostChildPageNum);

        // Recursively traverse all child pages in order
        for (size_t i = 0; i < page->cells.size(); i++) {
            const auto& cell = page->cells[i];

            LOG_DEBUG("  Interior Cell[" << i << "] - leftChild: " <<
                      (cell.leftChildPageNum.has_value() ? std::to_string(cell.leftChildPageNum.value()) : "NONE") <<
                      ", dataFormat size: " << cell.dataFormat.size());

            if (cell.leftChildPageNum.has_value()) {
                std::string cellKey = extractStringValue(cell, page->data, 0);
                LOG_DEBUG("    Interior cell key: '" << cellKey << "'");

                // IMPORTANT: Interior index cells contain the full key (including rowId)
                // Check if this cell's key matches our search
                if (cellKey == keyToSearchFor) {
                    long rowId = extractRowId(cell, page->data);
                    LOG_DEBUG("    *** MATCH FOUND IN INTERIOR CELL *** Cell[" << i << "] key='" << cellKey << "' rowId=" << rowId);
                    results.push_back(rowId);
                }

                LOG_DEBUG("    -> Descending to left child page " << cell.leftChildPageNum.value());
                traverseAndCollect(dbPath, cell.leftChildPageNum.value(), pageSize, keyToSearchFor, results);
            } else {
                LOG_DEBUG("    WARNING: Interior cell has no left child page number!");
            }
        }

        // Don't forget the rightmost child
        LOG_DEBUG("  -> Descending to rightmost child page " << page->rightMostChildPageNum);
        traverseAndCollect(dbPath, page->rightMostChildPageNum, pageSize, keyToSearchFor, results);
    }
}

std::vector<long> IndexScan::findRowId(std::string keyToSearchFor) {
    // Remove quotes if present
    if (!keyToSearchFor.empty() && keyToSearchFor.front() == '\'' && keyToSearchFor.back() == '\'') {
        keyToSearchFor = keyToSearchFor.substr(1, keyToSearchFor.length() - 2);
    }

    LOG_DEBUG("========================================");
    LOG_DEBUG("Starting IndexScan for key: '" << keyToSearchFor << "'");
    LOG_DEBUG("Root page: " << rootPageNum << ", Page size: " << pageSize);
    LOG_DEBUG("========================================");

    std::vector<long> matchingRowIds;

    // Traverse the entire index B-tree to find all matching entries
    traverseAndCollect(dbPath, rootPageNum, pageSize, keyToSearchFor, matchingRowIds);

    if (matchingRowIds.empty()) {
        throw std::runtime_error("Key not found in index: " + keyToSearchFor);
    }

    LOG_DEBUG("========================================");
    LOG_DEBUG("Search complete! Found " << matchingRowIds.size() << " matching row(s):");
    for (size_t i = 0; i < matchingRowIds.size(); i++) {
        LOG_DEBUG("  Match[" << i << "]: rowId = " << matchingRowIds[i]);
    }
    LOG_DEBUG("========================================");

    return matchingRowIds;
}
