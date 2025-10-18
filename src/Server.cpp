#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <unordered_map>

#include "SqliteFilePageReader.h"

#include "antlr4-runtime.h"
#include "DebugUtils.h"
#include "gen_sqlite/SQLiteLexer.h"
#include "gen_sqlite/SQLiteParser.h"
#include "SQLiteAstBuilder.hpp"
#include "TableScan.h"
#include "IndexScan.h"
#include "btree/SqlitePage.h"

static const int DATABASE_HEADER_SIZE_BYTES = 100;
static const int CELL_OFFSET = 3;
static const int PAGE_SIZE_OFFSET = 16;


enum Command {
    DBINFO, TABLES, INVALID
};

std::unique_ptr<btree::SqlitePage> loadPage(std::ifstream& stream, int pageNum, int pageSize) {
    stream.seekg((pageNum - 1) * pageSize, std::ios::beg);
    auto buffer = std::make_unique<uint8_t[]>(pageSize);
    stream.read(reinterpret_cast<char*>(buffer.get()), pageSize);

    return std::make_unique<btree::SqlitePage>(pageSize, pageNum, std::move(buffer));
}


int read2Bytes(std::ifstream& file, int offset) {
    file.seekg(offset, std::ios::beg);
    size_t n = 2;
    uint16_t value = 0;
    file.read(reinterpret_cast<char*>(&value), n);
    if (little_endian()) {
        value = swap(value);
    }
    file.close();
    return value;
}

int file_reader(const std::string& file_location) {
    //    std::cout << "Reading dbFile " << file_location << std::endl;
    std::ifstream file(file_location, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Error could not open dbFile");
    }
    int val = read2Bytes(file, PAGE_SIZE_OFFSET);
    return val;
}


int numCellsInFirstPage(const std::string& file_location) {
    //read dbFile in binary mode
    std::ifstream file(file_location, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("File is not open , I don't know why");
    }
    int offset = DATABASE_HEADER_SIZE_BYTES + CELL_OFFSET;
    int val = read2Bytes(file, offset);
    std::cout << "number of tables: " << val << std::endl;

    return val;
}

std::any parseSQL(const std::string& sql) {
    antlr4::ANTLRInputStream input(sql);
    SQLiteLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    SQLiteParser parser(&tokens);
    auto* tree = parser.parse(); // top-level rule for this grammar
    SqliteAstBuilder v;
    return v.visit(tree);
}

bool isPk(std::vector<std::shared_ptr<Constraint>> constraints) {
    for (const auto& constraint : constraints) {
        if (constraint->constraintType == ConstraintType::COLUMN_CONSTRAINT && constraint->autoIncrement == true) {
            return true;
        }
    }
    return false;
}

uint16_t readBigEndian16(std::ifstream& file, std::streampos offset) {
    uint8_t bytes[2];
    file.seekg(offset, std::ios::beg);
    file.read(reinterpret_cast<char*>(bytes), 2);
    return (static_cast<uint16_t>(bytes[0]) << 8) |
        (static_cast<uint16_t>(bytes[1]));
}

std::unique_ptr<btree::SqlitePage> schemaPage(std::ifstream& stream) {
    uint16_t pageSize = readBigEndian16(stream, 16);
    if (pageSize == 1)
        pageSize = 65536; // SQLite quirk: 1 means 65536

    return loadPage(stream, 1, pageSize);
}

int main(int argc, char* argv[]) {
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

    // You can use print statements as follows for debugging, they'll be visible when running tests.
    std::cerr << "Logs from your program will appear here" << std::endl;

    if (argc != 3) {
        std::cerr << "Expected two arguments" << std::endl;
        return 1;
    }

    std::string database_file_path = argv[1];
    std::string command = argv[2];

    std::unordered_map<std::string, Command> commandMap = {
        {".dbinfo", DBINFO},
        {".tables", TABLES}
    };

    Command c = commandMap.count(command) ? commandMap[command] : INVALID;

    std::ifstream stream(database_file_path);
    auto firstPage = schemaPage(stream);
    std::map<std::string, std::string> tableNames;
    std::map<std::string, int> rootPages;
    std::map<std::string, std::string> objectTypes; // Track if entry is table or index

    // Map from table name to map of column name to index info
    std::map<std::string, std::map<std::string, std::pair<std::string, int>>> tableIndexes;

    for (auto cell : firstPage->cells) {
        // Get type (table or index)
        std::tuple typeTuple = cell.dataFormat[0];
        int offset = std::get<2>(typeTuple);
        std::string objType = std::string(reinterpret_cast<char*>(firstPage->data.get()) + offset, std::get<1>(typeTuple));

        std::tuple createSqlTuple = cell.dataFormat[4];
        offset = std::get<2>(createSqlTuple);
        std::string createSql = std::string(reinterpret_cast<char*>(firstPage->data.get()) + offset, std::get<1>(createSqlTuple));

        std::tuple tableNameTuple = cell.dataFormat[1];
        offset = std::get<2>(tableNameTuple);
        std::string tableName = std::string(reinterpret_cast<char*>(firstPage->data.get()) + offset, std::get<1>(tableNameTuple));

        std::tuple rootPageTuple = cell.dataFormat[3];
        offset = std::get<2>(rootPageTuple);
        uint64_t value = 0;
        for (int i = 0; i < std::get<1>(rootPageTuple); ++i) {
            value = (value << 8) | static_cast<uint64_t>(firstPage->data.get()[offset + i]);
        }

        if (!tableName.starts_with("sqlite")) {
            tableNames[tableName] = createSql;
            rootPages[tableName] = value;
            objectTypes[tableName] = objType;

            // Parse index definitions to map them to tables and columns
            if (objType == "index" && createSql.find("CREATE INDEX") != std::string::npos) {
                // Parse: CREATE INDEX idx_name ON table_name (column_name)
                size_t onPos = createSql.find(" ON ");
                size_t parenPos = createSql.find("(", onPos);
                size_t closeParenPos = createSql.find(")", parenPos);

                if (onPos != std::string::npos && parenPos != std::string::npos) {
                    // Extract table name
                    std::string indexTableName = createSql.substr(onPos + 4, parenPos - (onPos + 4));
                    // Trim whitespace
                    indexTableName.erase(0, indexTableName.find_first_not_of(" \t\n\r"));
                    indexTableName.erase(indexTableName.find_last_not_of(" \t\n\r") + 1);

                    // Extract column name
                    std::string columnName = createSql.substr(parenPos + 1, closeParenPos - (parenPos + 1));
                    columnName.erase(0, columnName.find_first_not_of(" \t\n\r"));
                    columnName.erase(columnName.find_last_not_of(" \t\n\r") + 1);

                    // Store the index info: indexName -> rootPage
                    tableIndexes[indexTableName][columnName] = {tableName, value};
                }
            }
        }
    }

    switch (c) {
    case DBINFO: {
        std::ifstream database_file(database_file_path, std::ios::binary);
        if (!database_file) {
            std::cerr << "Failed to open the database dbFile" << std::endl;
            return 1;
        }

        int val = file_reader(database_file_path);
        std::cout << "database page size: " << val << std::endl;
        numCellsInFirstPage(database_file_path);
        break;
    }
    case TABLES: {
        for (auto table : tableNames) {
            std::cout << table.first << std::endl;
        }
        break;
    }
    default:
        std::string sql = command;
        auto node = parseSQL(sql);
        if (node.type() == typeid(std::shared_ptr<SelectStatement>)) {
            auto select = std::any_cast<std::shared_ptr<SelectStatement>>(node);
            if (select->countQuery) {
                auto tablePage = loadPage(stream, rootPages[select->fromTable->tableName], firstPage->pageSize);
                std::cout << tablePage->numCellsInPage << std::endl;
                return 0;
            }
            std::string tableName = select->fromTable->tableName;

            // Check if we can use an index for this query
            bool useIndex = false;
            std::string indexColumn;
            std::string indexValue;
            int indexRootPage = -1;

            if (select->whereClause.has_value()) {
                // Extract the WHERE clause column and value
                std::string whereColumn = select->whereClause.value()->left->value;
                std::string whereValue = select->whereClause.value()->right->value;

                // Remove quotes from value
                if (!whereValue.empty() && whereValue.front() == '\'' && whereValue.back() == '\'') {
                    whereValue = whereValue.substr(1, whereValue.length() - 2);
                }

                // Check if there's an index on this column
                if (tableIndexes.count(tableName) && tableIndexes[tableName].count(whereColumn)) {
                    useIndex = true;
                    indexColumn = whereColumn;
                    indexValue = whereValue;
                    indexRootPage = tableIndexes[tableName][whereColumn].second;
                    LOG_DEBUG("Using index on column '" << indexColumn << "' for value '" << indexValue << "'");
                }
            }

            // Create table scan - only load all pages if NOT using index
            TableScan scan = TableScan(tableName, rootPages[tableName], stream, firstPage->pageSize, !useIndex);
            scan.selectColumns = select->fromTable->columns;
            auto createTable = std::any_cast<CreateTableStatement>(parseSQL(tableNames[tableName]));
            for (auto& col : createTable.columns) {
                scan.tableColumns.emplace_back(col);
            }

            if (useIndex) {
                // Use index scan to find matching rowIds
                LOG_DEBUG("Performing index scan on index at root page " << indexRootPage);
                IndexScan indexScan(database_file_path, firstPage->pageSize, indexRootPage, {});

                try {
                    std::vector<long> matchingRowIds = indexScan.findRowId(indexValue);
                    LOG_DEBUG("Index scan found " << matchingRowIds.size() << " matching rows");

                    // Fetch and print each matching row by rowId
                    std::ifstream tableStream(database_file_path);
                    for (long rowId : matchingRowIds) {
                        scan.printRowByRowId(rowId, tableStream);
                    }
                } catch (const std::exception& e) {
                    LOG_DEBUG("Index scan failed: " << e.what() << ", falling back to table scan");
                    // Fall back to table scan if index lookup fails
                    scan.printTable(select->whereClause);
                }
            } else {
                // No index available, use full table scan
                LOG_DEBUG("No index available, performing full table scan");
                scan.printTable(select->whereClause);
            }
        }
    }
    return 0;
}
