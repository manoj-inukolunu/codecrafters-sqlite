//
// Created by Manoj on 14-10-2025.
//

#include "TableScan.h"

#include <map>

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
