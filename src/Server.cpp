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
#include "btree/SqlitePage.h"

static const int DATABASE_HEADER_SIZE_BYTES = 100;
static const int CELL_OFFSET = 3;
static const int PAGE_SIZE_OFFSET = 16;


enum Command {
    DBINFO, TABLES, INVALID
};

btree::SqlitePage loadPage(std::ifstream& stream, int pageNum, int pageSize) {
    stream.seekg((pageNum - 1) * pageSize, std::ios::beg);
    auto buffer = std::make_unique<uint8_t[]>(pageSize);
    stream.read(reinterpret_cast<char*>(buffer.get()), pageSize);

    return btree::SqlitePage(pageSize, pageNum, std::move(buffer));
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

btree::SqlitePage schemaPage(std::ifstream& stream) {
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
    for (auto cell : firstPage.cells) {
        std::tuple createSqlTuple = cell.dataFormat[4];
        int offset = std::get<2>(createSqlTuple);
        std::string createSql = std::string(reinterpret_cast<char*>(firstPage.data.get()) + offset, std::get<1>(createSqlTuple));

        std::tuple tableNameTuple = cell.dataFormat[1];
        offset = std::get<2>(tableNameTuple);
        std::string tableName = std::string(reinterpret_cast<char*>(firstPage.data.get()) + offset, std::get<1>(tableNameTuple));
        if (!tableName.starts_with("sqlite")) {
            tableNames[tableName] = createSql;
        }

        std::tuple rootPageTuple = cell.dataFormat[3];
        offset = std::get<2>(rootPageTuple);
        uint64_t value = 0;
        for (int i = 0; i < std::get<1>(rootPageTuple); ++i) {
            value = (value << 8) | static_cast<uint64_t>(firstPage.data.get()[offset + i]);
        }
        rootPages[tableName] = value;
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


        //print table names
        for (const auto& [name, sql] : tableNames) {
            LOG_INFO("Table Name: " << name << " SQL: " << sql);
        }

        if (node.type() == typeid(std::shared_ptr<SelectStatement>)) {
            auto select = std::any_cast<std::shared_ptr<SelectStatement>>(node);
            if (select->countQuery) {
                auto tablePage = loadPage(stream, rootPages[select->fromTable->tableName], firstPage.pageSize);
                std::cout << tablePage.numCellsInPage << std::endl;
                return 0;
            }
            LOG_INFO("Reading Table" << select->fromTable->tableName);
            LOG_INFO("Column Names ");
            int idColumnIndex = -1;
            for (auto column : select->fromTable->columns) {
                LOG_INFO(column.name);
            }
            auto it = tableNames.find(select->fromTable->tableName);
            LOG_INFO("Create SQL " << it->second);

            auto createNode = parseSQL(it->second);
            std::map<std::string, std::pair<int, std::shared_ptr<ColumnDefinition>>> columnMap;
            if (createNode.type() == typeid(CreateTableStatement)) {
                auto createTable = std::any_cast<CreateTableStatement>(createNode);
                LOG_INFO("Table name: " << createTable.tableName);

                for (int i = 0; i < createTable.columns.size(); i++) {
                    columnMap[createTable.columns[i]->name] = {i, createTable.columns[i]};
                }
            }
            //print rootpage for table
            LOG_INFO("Root Page " << rootPages[select->fromTable->tableName]);
            auto tablePage = loadPage(stream, rootPages[select->fromTable->tableName], firstPage.pageSize);

            std::vector<int> cols;
            for (auto col : select->fromTable->columns) {
                cols.emplace_back(columnMap[col.name].first);
            }

            std::vector<std::vector<std::string>> cellRows;
            for (auto cell : tablePage.cells) {
                std::vector<std::string> currentRow = tablePage.collectColumnData(cell);
                cellRows.emplace_back(currentRow);
            }
            for (int cellRow = 0; cellRow < cellRows.size(); cellRow++) {
                auto row = cellRows[cellRow];
                bool shouldPrint = false;
                std::vector<std::string> toPrint;
                for (int i = 0; i < select->fromTable->columns.size(); i++) {
                    std::string currColName = select->fromTable->columns[i].name;
                    if (select->whereClause.has_value()) {
                        std::string colName = select->whereClause->get()->left->value;
                        std::string colValue = select->whereClause->get()->right->value;
                        int colId = columnMap[currColName].first;
                        std::string currColValue = "'" + row[colId] + "'";
                        if (colName == currColName && currColValue == colValue) {
                            shouldPrint = true;
                        }
                    }
                    int colId = columnMap[currColName].first;
                    toPrint.emplace_back(row[colId]);
                    if (i != select->fromTable->columns.size() - 1) {
                        toPrint.emplace_back("|");
                    }
                }
                if (shouldPrint) {
                    for (auto s : toPrint) {
                        std::cout << s;
                    }
                    std::cout << std::endl;
                }
                toPrint.clear();
            }
        }
    }
    return 0;
}
