#pragma once

#include "SQLiteParserBaseVisitor.h"
#include <iostream>

#include "Node.h"


enum class StatementType {
    SELECT, CREATE_TABLE
};


class SqliteVisitor : public SQLiteParserBaseVisitor {
public:
    StatementType statementType;

    std::string statementTypeToString() const {
        switch (statementType) {
        case StatementType::SELECT:
            return "SELECT";
        case StatementType::CREATE_TABLE:
            return "CREATE TABLE";
        default:
            return "INVALID";
        }
    }

    static inline std::string upper(std::string s) {
        std::transform(s.begin(), s.end(), s.begin(),
                       [](unsigned char c) {
                           return std::toupper(c);
                       });
        return s;
    }

    std::vector<std::string> tables;
    bool countStarQuery = false;
    bool selectQuery = false;


    std::any visitSchema_name(SQLiteParser::Schema_nameContext* context) override {
        visit(context->any_name());

        return visitChildren(context);
    }

    std::any visitAny_name(SQLiteParser::Any_nameContext* context) override {
        context->getText();

        return visitChildren(context);
    }

    std::any visitKeyword(SQLiteParser::KeywordContext* context) override {
        context->getText();
        return visitChildren(context);
    }

    std::any visitSql_stmt(SQLiteParser::Sql_stmtContext* ctx) override {
        if (auto stmt = ctx->create_table_stmt()) {
            statementType = StatementType::CREATE_TABLE;
        } else if (auto stmt = ctx->select_stmt()) {
            statementType = StatementType::SELECT;
        }
        return visitChildren(ctx);
    }

    std::any visitSelect_core(SQLiteParser::Select_coreContext* context) override {
        selectQuery = true;
        return visitChildren(context);
    }

    std::any visitTable_or_subquery(SQLiteParser::Table_or_subqueryContext* ctx) override {
        if (auto tblName = ctx->table_name()) {
            tables.emplace_back(ctx->table_name()->getText());
        }
        return antlr4::tree::AbstractParseTreeVisitor::visitChildren(ctx);
    }

    std::any visitColumn_constraint(SQLiteParser::Column_constraintContext* ctx) override {
        CreateTable createTable;

        return visitChildren(ctx);
    }

    std::any visitCreate_table_stmt(SQLiteParser::Create_table_stmtContext* context) override {
        std::vector<SQLiteParser::Column_defContext*> ctx = context->column_def();

        for (int i = 0; i < ctx.size(); i++) {
            std::cout << ctx[i]->column_name()->getText() << std::endl;
            std::cout << ctx[i]->type_name()->getText() << std::endl;
            for (int j = 0; j < ctx[i]->column_constraint().size(); j++) {
                std::cout << ctx[i]->column_constraint()[j]->getText() << std::endl;
            }
        }
        return visitChildren(context);
    }


    std::any visitQualified_table_name(SQLiteParser::Qualified_table_nameContext* ctx) override {
        if (auto tn = ctx->table_name()) {
            tables.emplace_back(tn->getText());
        }

        return antlr4::tree::AbstractParseTreeVisitor::visitChildren(ctx);
    }

    std::any visitExpr(SQLiteParser::ExprContext* ctx) override {
        if (auto fn = ctx->function_name()) {
            const std::string name = upper(fn->getText()); // e.g., "COUNT"

            static const std::unordered_set<std::string> kAggregates = {"COUNT"};

            if (kAggregates.count(name)) {
                const bool star = (ctx->STAR() != nullptr); // COUNT(*)
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
