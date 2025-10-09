//
// Created by Manoj on 09-10-2025.
//

#include "SQLiteParserBaseVisitor.h"


#include "catch_amalgamated.hpp"
#include "SQLiteLexer.h"
#include "SQLitePrinter.hpp"


TEST_CASE("Where") {
    std::string sql = "SELECT name, color FROM apples WHERE color = 'Yellow'";

    antlr4::ANTLRInputStream input(sql);
    SQLiteLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    SQLiteParser parser(&tokens);

    auto* tree = parser.parse(); // top-level rule for this grammar

    SqliteVisitor v;

    std::any node = v.visit(tree);
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

    SqliteVisitor v;
    CreateTableStatement statement = std::any_cast<CreateTableStatement>(v.visit(tree));

    std::cout << "Table name: " << statement.tableName << std::endl;

    REQUIRE(statement.tableName =="apples");
}
