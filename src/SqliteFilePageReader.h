//
// Created by Manoj Inukolunu on 28/09/25.
//

#ifndef SQLITE_STARTER_CPP_SQLITESCHEMAPAGEREADER_H
#define SQLITE_STARTER_CPP_SQLITESCHEMAPAGEREADER_H

#include <iostream>
#include <fstream>
#include <cstdint>
#include <vector>

#include "utils.h"
#include <cassert>
#include <map>
#include <any>
#include "common/SqliteUtils.h"
#include "catalog/CatalogBuilder.h"


/**
 * Schema Page is the first page of the file
 */

#include <memory>

#include "SqliteFilePageReader.h"


struct SqliteSchemaTables {
    std::string tableName;
    std::string type;
    std::string sql;
    std::string name;
    int rootPage;

    void toString() const {
        std::cout << "SqliteObject Name " << tableName << " type " << type << " sql " << sql << " rootPage " << rootPage
            << " name " << name << std::endl;
    }
};

struct SqliteBTreeSchemaCell {
    struct RecordHeader {
        varint headerSize;
        std::vector<RecordColumn> recordColumns;
    };

    int cellNumber{};
    //offset is from the beginning of the file
    FileOffset offset{};
    varint recordSize{};
    varint rowId{};
    RecordHeader record;
    std::map<std::string, std::any> schema;
    FileOffset recordBodyOffset{};
};


class SqliteFilePageReader {
public:
    SqliteFilePageReader(int pageNum, const std::string& filePath);

    std::map<int, std::unique_ptr<std::uint8_t[]>> pageCache;
    std::string filePath;
    //If currentPage == 1 then the first 100 bytes are the database dbFile header
    int currentPage;
    BTreePageType pageType;
    int firstFreeBlockStart;
    int numCellsInPage;
    int cellContentAreaStart;
    int fragmentedFreeBytesInCellContentArea;
    //Appears only in the header of the interior b-tree page and is omitted for all other pages
    FileOffset rightMostPointer;
    std::vector<SqliteBTreeSchemaCell> cells;

    std::vector<SqliteSchemaTables> tables;

    std::vector<FileOffset> cellContentOffsets;

    void printTableNames();

    void buildSchemaTableRows();

    void loadPage(int pageNum);

private:
    // The database dbFile
    std::unique_ptr<std::ifstream> dbFile;
    uint16_t pageSize;

    std::vector<std::string> tableNames;
    std::vector<std::pair<FileOffset, RecordColumn>> recordDataOffsets;


    void buildCellBody(SqliteBTreeSchemaCell& cell);

    void buildCell(int pageNum, SqliteBTreeSchemaCell& cell);

    void buildSqliteSchemaTable();

    void parseHeader(int pageNum);

    void processCellPointers(int pageNum);

    void processAllCells(int pageNum);

    uint16_t readPageSize();
    FileOffset pageBegin;
};


#endif //SQLITE_STARTER_CPP_SQLITESCHEMAPAGEREADER_H
