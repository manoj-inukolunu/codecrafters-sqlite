#pragma once

#include "SQLiteParserBaseVisitor.h"
#include <iostream>

class SqliteVisitor : public SQLiteParserBaseVisitor {
public:

    static inline std::string upper(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) { return std::toupper(c); });
        return s;
    }

    std::vector<std::string> tables;
    bool countStarQuery = false;

    std::any visitTable_or_subquery(SQLiteParser::Table_or_subqueryContext *ctx) override {
        if (auto tblName = ctx->table_name()) {
            tables.emplace_back(ctx->table_name()->getText());
        }
        return antlr4::tree::AbstractParseTreeVisitor::visitChildren(ctx);
    }


    std::any visitQualified_table_name(SQLiteParser::Qualified_table_nameContext *ctx) override {
        if (auto tn = ctx->table_name()) {
            tables.emplace_back(tn->getText());
        }

        return antlr4::tree::AbstractParseTreeVisitor::visitChildren(ctx);
    }

    std::any visitExpr(SQLiteParser::ExprContext *ctx) override {
        if (auto fn = ctx->function_name()) {
            const std::string name = upper(fn->getText());  // e.g., "COUNT"

            static const std::unordered_set<std::string> kAggregates = {"COUNT"};

            if (kAggregates.count(name)) {
                const bool star = (ctx->STAR() != nullptr);        // COUNT(*)
                if (name == "COUNT" && star) {
                    countStarQuery = true;
                } else {
                    throw std::runtime_error("Nothing else is implemented other than count");
                }
            }
        }
        return visitChildren(ctx);
    }

};
