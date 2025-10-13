//
// Created by Manoj on 09-10-2025.
//

#include "SQLiteParserBaseVisitor.h"


#include "catch_amalgamated.hpp"
#include "SQLiteLexer.h"
#include "SQLiteAstBuilder.hpp"
#include "SqliteFilePageReader.h"
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

btree::SqlitePage loadPage(std::ifstream& stream, int pageNum, int pageSize) {
    stream.seekg((pageNum - 1) * pageSize, std::ios::beg);
    auto buffer = std::make_unique<uint8_t[]>(pageSize);
    stream.read(reinterpret_cast<char*>(buffer.get()), pageSize);

    return btree::SqlitePage(pageSize, pageNum, std::move(buffer));
}

bool isPk(const std::vector<Constraint>& constraints) {
    for (const auto& constraint : constraints) {
        if (constraint.constraintType == ConstraintType::COLUMN_CONSTRAINT && ) {
            return true;
        }
    }
    return false;
}

TEST_CASE("Read Data From a Single Column") {
    std::cerr.rdbuf(std::cerr.rdbuf());
    std::string sql = "select id from oranges";
    auto node = parseSQL(sql);
    std::ifstream stream("/mnt/c/Users/Manoj/Projects/code-crafters/codecrafters-sqlite-cpp/sample.db");

    uint16_t pageSize = readBigEndian16(stream, 16);
    if (pageSize == 1)
        pageSize = 65536; // SQLite quirk: 1 means 65536

    auto page = loadPage(stream, 1, pageSize);
    std::map<std::string, std::string> tableNames;
    std::map<std::string, int> rootPages;
    for (auto cell : page.cells) {
        std::tuple createSqlTuple = cell.dataFormat[4];
        int offset = std::get<2>(createSqlTuple);
        std::string createSql = std::string(reinterpret_cast<char*>(page.data.get()) + offset, std::get<1>(createSqlTuple));

        std::tuple tableNameTuple = cell.dataFormat[1];
        offset = std::get<2>(tableNameTuple);
        std::string tableName = std::string(reinterpret_cast<char*>(page.data.get()) + offset, std::get<1>(tableNameTuple));
        if (!tableName.starts_with("sqlite")) {
            tableNames[tableName] = createSql;
        }

        std::tuple rootPageTuple = cell.dataFormat[3];
        offset = std::get<2>(rootPageTuple);
        uint64_t value = 0;
        for (int i = 0; i < std::get<1>(rootPageTuple); ++i) {
            value = (value << 8) | static_cast<uint64_t>(page.data.get()[offset + i]);
        }
        rootPages[tableName] = value;
    }


    //print table names
    for (const auto& [name, sql] : tableNames) {
        std::cerr << "Table Name: " << name << " SQL: " << sql << std::endl;
    }

    if (node.type() == typeid(std::shared_ptr<SelectStatement>)) {
        auto select = std::any_cast<std::shared_ptr<SelectStatement>>(node);
        std::cerr << "Reading Table" << select->fromTable->tableName << std::endl;
        std::cerr << "Column Names " << std::endl;
        int idColumnIndex = -1;
        for (auto column : select->fromTable->columns) {
            std::cerr << column.name << std::endl;
        }
        //only one table is supported
        REQUIRE(select->fromTable->tableName == "oranges");
        auto it = tableNames.find(select->fromTable->tableName);
        REQUIRE(it != tableNames.end());
        std::cerr << "Create SQL " << it->second << std::endl;

        auto createNode = parseSQL(it->second);
        std::map<std::string, std::pair<int, ColumnDefinition>> columnMap;
        if (createNode.type() == typeid(CreateTableStatement)) {
            auto createTable = std::any_cast<CreateTableStatement>(createNode);
            std::cerr << "Table name: " << createTable.tableName << std::endl;

            for (int i = 0; i < createTable.columns.size(); i++) {
                columnMap[createTable.columns[i].name] = {i, createTable.columns[i]};
            }
            REQUIRE(createTable.tableName == "oranges");
        }
        int colOrder = columnMap[select->fromTable->columns[0].name].first;

        //print rootpage for table
        std::cerr << "Root Page " << rootPages[select->fromTable->tableName] << std::endl;
        REQUIRE(rootPages[select->fromTable->tableName] == 4);
        auto tablePage = loadPage(stream, rootPages[select->fromTable->tableName], pageSize);

        std::cerr << "Col Order :" << colOrder << std::endl;
        for (auto cell : tablePage.cells) {
            if (isPk(columnMap[select->fromTable->columns[0].name].second.constraints)) {
                //primary key is always the rowid
                std::cerr << "Row Id " << cell.rowId << std::endl;
                REQUIRE(cell.rowId == 1 || cell.rowId == 2 || cell.rowId == 3);
            } else {
                tablePage.printColumn(cell, colOrder);
            }
        }


        //load page by rootpage
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
    std::cerr << "Total pages: " << totalPages << "\n";
    std::cerr << "Page size: " << pageSize << " bytes\n";


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

            std::cerr << page.numCellsInPage << std::endl;
            for (auto cell : page.cells)
                page.printAllCellData(cell);
        } catch (std::exception& e) {
            std::cerr << e.what() << " " << i << std::endl;
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
