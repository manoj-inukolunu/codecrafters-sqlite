//
// Created by Manoj Inukolunu on 04/10/25.
//
#include "antlr4-runtime.h"
#include "gen_sqlite/SQLiteLexer.h"
#include "gen_sqlite/SQLiteParser.h"
#include "SQLitePrinter.hpp"

int main() {
    const std::string sql = R"SQL(
    SELECT COUNT(*) FROM apples;
  )SQL";

    antlr4::ANTLRInputStream input(sql);
    SQLiteLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    SQLiteParser parser(&tokens);

    auto *tree = parser.parse(); // top-level rule for this grammar

    SqliteVisitor v;
    v.visit(tree);
    for (auto table: v.tables) {
        if (v.countStarQuery)
            std::cout << table << std::endl;
    }

    return 0;
}
