#pragma once

#include "SQLiteParserBaseVisitor.h"
#include <iostream>

#include "Node.h"
#include "parser/CreateTableStatement.h"


class SqliteVisitor : public SQLiteParserBaseVisitor {
public:
    StatementType statementType;

    std::string statementTypeToString() const {
        switch (statementType) {
        case StatementType::SELECT_STATEMENT:
            return "SELECT";
        case StatementType::CREATE_STATEMENT:
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
        context->any_name()->getText();

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
            statementType = StatementType::CREATE_STATEMENT;
        } else if (auto stmt = ctx->select_stmt()) {
            statementType = StatementType::SELECT_STATEMENT;
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
        return visitChildren(ctx);
    }

    std::any visitColumn_def(SQLiteParser::Column_defContext* ctx) override {
        ColumnDefinition definition(ctx->column_name()->getText(), dataTypeFromString(ctx->type_name()->getText()));
        return definition;
    }

    std::any visitCreate_table_stmt(SQLiteParser::Create_table_stmtContext* context) override {
        std::vector<SQLiteParser::Column_defContext*> ctx = context->column_def();

        if (ctx.size() == 0) {
            throw std::runtime_error("No column definition found . There must be columns for creating");
        }


        CreateTableStatement createTable(StatementType::CREATE_STATEMENT);
        if (context->schema_name()) {
            createTable.schemaName = context->schema_name()->getText();
        }
        createTable.tableName = context->table_name()->getText();
        createTable.isCreateAsSelect = context->select_stmt();
        createTable.isTemporary = context->TEMP_() || context->TEMPORARY_();
        createTable.withoutRowId = context->WITHOUT_();

        std::vector<ColumnDefinition> columns;
        for (int i = 0; i < ctx.size(); i++) {
            columns.emplace_back(std::any_cast<ColumnDefinition>(visit(ctx[i])));
        }
        createTable.columns = columns;

        return createTable;
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
