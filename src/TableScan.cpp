//
// Created by Manoj on 14-10-2025.
//

#include "TableScan.h"

#include <map>
#include <optional>
#include <functional>

#include "parser/ParsedExpression.h"


void TableScan::printTable(std::optional<std::shared_ptr<ParsedExpression>> whereClause) const {
    std::map<std::string, int> colIdxMap;
    std::map<std::string, std::shared_ptr<ColumnDefinition>> colDefMap;
    for (int i = 0; i < tableColumns.size(); i++) {
        colIdxMap[tableColumns[i]->name] = i;
    }

    // print all the rows in the table
    // Col1,Col2,Col3 ... ColN
    for (auto& page : pages) {
        for (auto cell : page->cells) {
            std::vector<std::string> data;
            std::vector<std::string> cellData = page->collectColumnData(cell);
            bool shouldPrint = !whereClause.has_value();
            for (int i = 0; i < selectColumns.size(); i++) {
                auto col = selectColumns[i];
                if (whereClause.has_value()) {
                    std::string colName = whereClause.value()->left->value;
                    std::string colValue = whereClause.value()->right->value;
                    if ("'" + cellData[colIdxMap[colName]] + "'" == colValue) {
                        shouldPrint = true;
                    }
                }

                data.emplace_back(cellData[colIdxMap[col.name]]);
            }
            if (shouldPrint) {
                for (int i = 0; i < data.size(); i++) {
                    auto colValue = data[i];
                    if (i != data.size() - 1)
                        std::cout << colValue << "|";
                    else
                        std::cout << colValue;
                }
                std::cout << std::endl;
            }
        }
    }
}

std::unique_ptr<btree::SqlitePage> TableScan::loadPage(std::ifstream& stream, int pageNum, int pageSize) {
    stream.seekg((pageNum - 1) * pageSize, std::ios::beg);
    auto buffer = std::make_unique<uint8_t[]>(pageSize);
    stream.read(reinterpret_cast<char*>(buffer.get()), pageSize);

    return std::make_unique<btree::SqlitePage>(pageSize, pageNum, std::move(buffer));
}

void TableScan::traverseInOrder(std::unique_ptr<btree::SqlitePage> rootPage, std::ifstream& stream) {
    if (rootPage->pageType == LEAF_TABLE_PAGE) {
        pages.emplace_back(std::move(rootPage));
        return;
    }

    for (auto cell : rootPage->cells) {
        auto left = loadPage(stream, cell.leftChildPageNum.value(), rootPage->pageSize);
        traverseInOrder(std::move(left), stream);
    }

    auto right = loadPage(stream, rootPage->rightMostChildPageNum, rootPage->pageSize);
    traverseInOrder(std::move(right), stream);
}

// Helper function to find a cell by rowId in a B-tree
btree::Cell* TableScan::findCellByRowId(std::ifstream& stream, int pageNum, long targetRowId) const {
    auto page = loadPage(stream, pageNum, pageSize);

    if (page->pageType == LEAF_TABLE_PAGE) {
        // Search leaf page for the rowId
        for (auto& cell : page->cells) {
            if (cell.rowId == targetRowId) {
                // Found it! We need to return the cell, but it will be destroyed when page goes out of scope
                // So we'll use a different approach - store the cell data
                return &cell;
            }
        }
        return nullptr; // Not found in this leaf
    } else if (page->pageType == INTERIOR_TABLE_PAGE) {
        // Interior page - need to navigate to the correct child
        // Interior cells are sorted by rowId
        for (size_t i = 0; i < page->cells.size(); i++) {
            const auto& cell = page->cells[i];
            if (targetRowId <= cell.rowId) {
                // Target should be in the left subtree
                return findCellByRowId(stream, cell.leftChildPageNum.value(), targetRowId);
            }
        }
        // If we get here, target is > all cell rowIds, so check rightmost child
        return findCellByRowId(stream, page->rightMostChildPageNum, targetRowId);
    }

    return nullptr;
}

void TableScan::printRowByRowId(long rowId, std::ifstream& stream) const {
    // Build column index map
    std::map<std::string, int> colIdxMap;
    for (int i = 0; i < tableColumns.size(); i++) {
        colIdxMap[tableColumns[i]->name] = i;
    }

    // Helper to search recursively and return cell data
    std::function<std::optional<std::vector<std::string>>(int, long)> findAndExtractCell;
    findAndExtractCell = [&](int pageNum, long targetRowId) -> std::optional<std::vector<std::string>> {
        auto currentPage = loadPage(stream, pageNum, pageSize);

        if (currentPage->pageType == LEAF_TABLE_PAGE) {
            // Search leaf page for the rowId
            for (auto& cell : currentPage->cells) {
                if (cell.rowId == targetRowId) {
                    // Found it! Extract the data before the page goes out of scope
                    return currentPage->collectColumnData(cell);
                }
            }
            return std::nullopt;
        } else if (currentPage->pageType == INTERIOR_TABLE_PAGE) {
            // Navigate to the correct child
            for (size_t i = 0; i < currentPage->cells.size(); i++) {
                const auto& cell = currentPage->cells[i];
                if (targetRowId <= cell.rowId) {
                    return findAndExtractCell(cell.leftChildPageNum.value(), targetRowId);
                }
            }
            // Target is > all cell rowIds, check rightmost child
            return findAndExtractCell(currentPage->rightMostChildPageNum, targetRowId);
        }
        return std::nullopt;
    };

    auto cellDataOpt = findAndExtractCell(rootPageNum, rowId);
    if (!cellDataOpt.has_value()) {
        // Row not found - skip it
        return;
    }

    auto cellData = cellDataOpt.value();
    std::vector<std::string> data;

    for (int i = 0; i < selectColumns.size(); i++) {
        auto col = selectColumns[i];
        data.emplace_back(cellData[colIdxMap[col.name]]);
    }

    for (int i = 0; i < data.size(); i++) {
        auto colValue = data[i];
        if (i != data.size() - 1)
            std::cout << colValue << "|";
        else
            std::cout << colValue;
    }
    std::cout << std::endl;
}
