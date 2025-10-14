//
// Created by Manoj on 09-10-2025.
//

#include "SQLiteParserBasevisitor.h"
#include "../src/DebugUtils.h"


#include "catch_amalgamated.hpp"
#include "SQLiteLexer.h"
#include "SQLiteAstBuilder.hpp"
#include "SqliteFilePageReader.h"
#include "TableScan.h"
#include "btree/SqlitePage.h"


uint16_t readBigEndian16(std::ifstream& file, std::streampos offset) {
    uint8_t bytes[2];
    file.seekg(offset, std::ios::beg);
    file.read(reinterpret_cast<char*>(bytes), 2);
    return (static_cast<uint16_t>(bytes[0]) << 8) |
        (static_cast<uint16_t>(bytes[1]));
}

uint32_t readBigEndian32(std::ifstream& file, std::streampos offset) {
    uint8_t bytes[4];
    file.seekg(offset, std::ios::beg);
    file.read(reinterpret_cast<char*>(bytes), 4);
    return (static_cast<uint32_t>(bytes[0]) << 24) |
        (static_cast<uint32_t>(bytes[1]) << 16) |
        (static_cast<uint32_t>(bytes[2]) << 8) |
        (static_cast<uint32_t>(bytes[3]));
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

std::unique_ptr<btree::SqlitePage> loadPage(std::ifstream& stream, int pageNum, int pageSize) {
    stream.seekg((pageNum - 1) * pageSize, std::ios::beg);
    auto buffer = std::make_unique<uint8_t[]>(pageSize);
    stream.read(reinterpret_cast<char*>(buffer.get()), pageSize);

    return std::make_unique<btree::SqlitePage>(pageSize, pageNum, std::move(buffer));
}

void dfs(std::unique_ptr<btree::SqlitePage> page, std::ifstream& stream, std::vector<std::unique_ptr<btree::SqlitePage>>& pages) {
    if (page->pageType == LEAF_TABLE_PAGE) {
        pages.emplace_back(std::move(page));
        LOG_DEBUG("Leaf Page Encountered");
        return;
    }
    LOG_DEBUG("Loading Page " << page->pageNum);
    for (auto cell : page->cells) {
        auto left = loadPage(stream, cell.leftChildPageNum.value(), page->pageSize);
        dfs(std::move(left), stream, pages);
    }

    auto right = loadPage(stream, page->rightMostChildPageNum, page->pageSize);
    dfs(std::move(right), stream, pages);
}

TEST_CASE("Multi Page Read") {
    auto file = "/mnt/c/Users/Manoj/Projects/codecrafters-sqlite-cpp/superheroes.db";
    std::ifstream stream(file);
    uint16_t pageSize = readBigEndian16(stream, 16);
    auto page = loadPage(stream, 1, pageSize);
    std::map<std::string, std::string> tableNames;
    std::map<std::string, int> rootPages;
    for (auto cell : page->cells) {
        std::tuple createSqlTuple = cell.dataFormat[4];
        int offset = std::get<2>(createSqlTuple);
        std::string createSql = std::string(reinterpret_cast<char*>(page->data.get()) + offset, std::get<1>(createSqlTuple));

        std::tuple tableNameTuple = cell.dataFormat[1];
        offset = std::get<2>(tableNameTuple);
        std::string tableName = std::string(reinterpret_cast<char*>(page->data.get()) + offset, std::get<1>(tableNameTuple));
        if (!tableName.starts_with("sqlite")) {
            tableNames[tableName] = createSql;
        }

        std::tuple rootPageTuple = cell.dataFormat[3];
        offset = std::get<2>(rootPageTuple);
        uint64_t value = 0;
        for (int i = 0; i < std::get<1>(rootPageTuple); ++i) {
            value = (value << 8) | static_cast<uint64_t>(page->data.get()[offset + i]);
        }
        rootPages[tableName] = value;
    }

    std::string sql = "select id,name from superheroes ";
    auto node = parseSQL(sql);
    auto select = std::any_cast<std::shared_ptr<SelectStatement>>(node);

    auto createNode = parseSQL(tableNames["superheroes"]);
    auto createTable = std::any_cast<CreateTableStatement>(createNode);
    TableScan scan("superheroes", rootPages["superheroes"], stream, pageSize);
    scan.selectColumns = select->fromTable->columns;
    //set the scan table columns to the column definitions
    for (auto& col : createTable.columns) {
        scan.tableColumns.emplace_back(col);
    }

    scan.printTable(select->whereClause);
}

TEST_CASE("Select Count(*) ") {
    auto sql = "select count(*) from apples";
    auto node = parseSQL(sql);
    if (node.type() == typeid(std::shared_ptr<SelectStatement>)) {
        auto select = std::any_cast<std::shared_ptr<SelectStatement>>(node);
        std::string tableName = select->fromTable->tableName;
        //Find Root Page for Table


        REQUIRE(select->countQuery);
    }
}

TEST_CASE("Read Data From multiple Columns") {
    // Enable logging
    std::string sql = "select name from apples";
    auto node = parseSQL(sql);
    std::ifstream stream("/mnt/c/Users/Manoj/Projects/codecrafters-sqlite-cpp/sample.db");

    uint16_t pageSize = readBigEndian16(stream, 16);
    if (pageSize == 1)
        pageSize = 65536; // SQLite quirk: 1 means 65536

    auto page = loadPage(stream, 1, pageSize);
    std::map<std::string, std::string> tableNames;
    std::map<std::string, int> rootPages;
    for (auto cell : page->cells) {
        std::tuple createSqlTuple = cell.dataFormat[4];
        int offset = std::get<2>(createSqlTuple);
        std::string createSql = std::string(reinterpret_cast<char*>(page->data.get()) + offset, std::get<1>(createSqlTuple));

        std::tuple tableNameTuple = cell.dataFormat[1];
        offset = std::get<2>(tableNameTuple);
        std::string tableName = std::string(reinterpret_cast<char*>(page->data.get()) + offset, std::get<1>(tableNameTuple));
        if (!tableName.starts_with("sqlite")) {
            tableNames[tableName] = createSql;
        }

        std::tuple rootPageTuple = cell.dataFormat[3];
        offset = std::get<2>(rootPageTuple);
        uint64_t value = 0;
        for (int i = 0; i < std::get<1>(rootPageTuple); ++i) {
            value = (value << 8) | static_cast<uint64_t>(page->data.get()[offset + i]);
        }
        rootPages[tableName] = value;
    }


    //print table names
    for (const auto& [name, sql] : tableNames) {
        LOG_INFO("Table Name: " << name << " SQL: " << sql);
    }

    if (node.type() == typeid(std::shared_ptr<SelectStatement>)) {
        auto select = std::any_cast<std::shared_ptr<SelectStatement>>(node);
        LOG_INFO("Reading Table" << select->fromTable->tableName);
        LOG_INFO("Column Names ");
        int idColumnIndex = -1;
        for (auto column : select->fromTable->columns) {
            LOG_INFO(column.name);
        }
        //only one table is supported
        auto it = tableNames.find(select->fromTable->tableName);
        REQUIRE(it != tableNames.end());
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
        std::vector<int> cols;
        for (auto col : select->fromTable->columns) {
            cols.emplace_back(columnMap[col.name].first);
        }
        LOG_INFO("Root Page " << rootPages[select->fromTable->tableName]);
        REQUIRE(rootPages[select->fromTable->tableName] == 2);
        auto tablePage = loadPage(stream, rootPages[select->fromTable->tableName], pageSize);

        std::vector<std::vector<std::string>> cellValues;
        for (auto cell : tablePage->cells) {
            std::vector<std::string> currentRow = tablePage->collectColumnData(cell);
            cellValues.emplace_back(currentRow);
        }
        for (int j = 0; j < cellValues.size(); j++) {
            for (int i = 0; i < select->fromTable->columns.size(); i++) {
                int idx = columnMap[select->fromTable->columns[i].name].first;
                std::cout << cellValues[j][idx];
                if (i != select->fromTable->columns.size() - 1) {
                    std::cout << "|";
                }
            }
            std::cout << std::endl;
        }
    }
}

TEST_CASE("Sqlite Page Test") {
    // 1. Load root page
    // 2. Get all cells , each cell is a row
    // 3. For Each cell
    //          read datatypes [(datatype),(datatype),(datatype),....]
    //          read content [(value),(value) , ....]
    // 4. Apply the schema after reading ,get the column name from the create table statement.
    // the index of the column is the same as the index of the content.

    std::ifstream stream("/mnt/c/Users/Manoj/Projects/codecrafters-sqlite-cpp/sample.db");

    uint16_t pageSize = readBigEndian16(stream, 16);
    if (pageSize == 1)
        pageSize = 65536; // SQLite quirk: 1 means 65536


    uint32_t totalPages = readBigEndian32(stream, 28);
    LOG_INFO("Total pages: " << totalPages);
    LOG_INFO("Page size: " << pageSize << " bytes");


    if (!stream) {
        throw std::runtime_error("Cannot open SQLite file");
    }
    for (int i = 0; i < totalPages; i++) {
        try {
            int pageNum = i;
            stream.seekg(pageNum * pageSize, std::ios::beg);
            auto buffer = std::make_unique<uint8_t[]>(pageSize);
            stream.read(reinterpret_cast<char*>(buffer.get()), pageSize);

            btree::SqlitePage page(pageSize, pageNum + 1, std::move(buffer));

            LOG_INFO(page.numCellsInPage);
        } catch (std::exception& e) {
            LOG_ERROR("Error: " << e.what() << " at page " << i);
        }
    }
}


TEST_CASE("Read Page") {
    std::string sql = "select name from apples";
    antlr4::ANTLRInputStream input(sql);
    SQLiteLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    SQLiteParser parser(&tokens);
    auto* tree = parser.parse(); // top-level rule for this grammar
    SqliteAstBuilder v;
    auto node = v.visit(tree);

    if (node.type() == typeid(std::shared_ptr<SelectStatement>)) {
        auto select = std::any_cast<std::shared_ptr<SelectStatement>>(node);


        SqliteFilePageReader reader(1, "/mnt/c/Users/Manoj/Projects/codecrafters-sqlite-cpp/sample.db");
        reader.buildSchemaTableRows();

        std::string tableName = select->fromTable->tableName;

        REQUIRE(select->fromTable->tableName == "apples");


        auto it = std::find_if(reader.tables.begin(), reader.tables.end(),
                               [tableName](SqliteSchemaTables x) {
                                   return x.tableName == tableName;
                               });

        std::cerr << "Table " << tableName << " has create SQL " << it->sql << std::endl;
        std::cerr << "Table " << tableName << " has create root page " << it->rootPage << std::endl;

        reader.loadPage(2);


        REQUIRE(it->rootPage == 2);
    } else {
        throw std::runtime_error("Parsing failed only create table statement is implemented");
    }
}


TEST_CASE("Where") {
    std::string sql = "SELECT name, color FROM apples WHERE color = 'Yellow'";

    antlr4::ANTLRInputStream input(sql);
    SQLiteLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    SQLiteParser parser(&tokens);

    auto* tree = parser.parse(); // top-level rule for this grammar

    SqliteAstBuilder v;

    auto node = std::any_cast<std::shared_ptr<SelectStatement>>(v.visit(tree));

    std::cerr << statementTypeToString(node->statementType) << std::endl;

    if (node->whereClause) {
        std::shared_ptr<ParsedExpression> where = node->whereClause.value();
        std::cerr << where->left->value << " " << where->right->value << std::endl;
    };

    std::cerr << node->fromTable->tableName << std::endl;

    for (auto column : node->fromTable->columns) {
        std::cerr << column.name << std::endl;
    }

    std::cerr << "Testing " << std::endl;
}

TEST_CASE("smoke") {
    std::string sql = R"(
CREATE TABLE apples
(
    id integer primary key autoincrement,
    name text,
    color text
)
)";

    std::cerr << sql << std::endl;

    antlr4::ANTLRInputStream input(sql);
    SQLiteLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    SQLiteParser parser(&tokens);

    auto* tree = parser.parse(); // top-level rule for this grammar

    SqliteAstBuilder v;
    CreateTableStatement statement = std::any_cast<CreateTableStatement>(v.visit(tree));

    std::cerr << "Table name: " << statement.tableName << std::endl;

    REQUIRE(statement.tableName =="apples");
}
