//
// Created by Manoj on 09-10-2025.
//

#include "SQLiteParserBaseVisitor.h"


#include "catch_amalgamated.hpp"
#include "SQLiteLexer.h"
#include "SQLiteAstBuilder.hpp"
#include "SqliteFilePageReader.h"


static const int PAGE_SIZE_OFFSET = 16;

TEST_CASE("Read Table") {
    // 1. Load root page
    // 2. Get all cells , each cell is a row
    // 3. For Each cell
    //          read datatypes [(datatype),(datatype),(datatype),....]
    //          read content [(value),(value) , ....]
    // 4. Apply the schema after reading ,get the column name from the create table statement.
    // the index of the column is the same as the index of the content.

    SqliteFilePageReader reader(4, "/mnt/c/Users/Manoj/Projects/codecrafters-sqlite-cpp/sample.db");
    std::cout << reader.numCellsInPage << std::endl;



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

        std::cout << "Table " << tableName << " has create SQL " << it->sql << std::endl;
        std::cout << "Table " << tableName << " has create root page " << it->rootPage << std::endl;

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

    std::cout << statementTypeToString(node->statementType) << std::endl;

    if (node->whereClause) {
        std::shared_ptr<ParsedExpression> where = node->whereClause.value();
        std::cout << where->left->value << " " << where->right->value << std::endl;
    };

    std::cout << node->fromTable->tableName << std::endl;

    for (auto column : node->fromTable->columns) {
        std::cout << column.name << std::endl;
    }

    std::cout << "Testing " << std::endl;
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

    std::cout << sql << std::endl;

    antlr4::ANTLRInputStream input(sql);
    SQLiteLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    SQLiteParser parser(&tokens);

    auto* tree = parser.parse(); // top-level rule for this grammar

    SqliteAstBuilder v;
    CreateTableStatement statement = std::any_cast<CreateTableStatement>(v.visit(tree));

    std::cout << "Table name: " << statement.tableName << std::endl;

    REQUIRE(statement.tableName =="apples");
}
