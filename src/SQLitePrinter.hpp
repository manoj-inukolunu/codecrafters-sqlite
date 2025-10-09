#pragma once

#include "SQLiteParserBaseVisitor.h"
#include <iostream>

#include "Node.h"
#include "parser/CreateTableStatement.h"
#include "parser/SelectStatement.h"


class SqliteVisitor : public SQLiteParserBaseVisitor {
public:
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

    std::any visitParse(SQLiteParser::ParseContext* context) override {
        if (context->sql_stmt_list().size() > 1) {
            throw std::runtime_error("Only One statement is accepted at a time and semi colon is not required");
        }

        std::any statement = visit(context->sql_stmt_list()[0]);

        if (statement.type() == typeid(CreateTableStatement)) {
            return std::any_cast<CreateTableStatement>(statement);
        }
        throw std::runtime_error("Parsing failed only create table statement is implemented");
    }

    std::any visitSql_stmt_list(SQLiteParser::Sql_stmt_listContext* context) override {
        if (context->SCOL().data()) {
            throw std::runtime_error("Only One statement is accepted at a time and semi colon is not requried");
        }
        std::any statement = visit(context->sql_stmt()[0]);

        if (statement.type() == typeid(CreateTableStatement)) {
            return std::any_cast<CreateTableStatement>(statement);
        }
        if (statement.type() == typeid(SelectStatement)) {
            return std::any_cast<SelectStatement>(statement);
        }
        throw std::runtime_error("Parsing failed only create table statement is implemented");
    }

    std::any visitSql_stmt(SQLiteParser::Sql_stmtContext* ctx) override {
        std::any node = visitChildren(ctx);
        return node;
    }

    std::any visitSelect_stmt(SQLiteParser::Select_stmtContext* context) override {
        if (context->common_table_stmt()) {
            throw std::runtime_error("Common Table expressions are not supported yet");
        }
        if (context->order_by_stmt() || context->limit_stmt()) {
            throw std::runtime_error("Order by or limit  is not supported yet");
        }

        return visitChildren(context);
    }

    std::any visitSelect_core(SQLiteParser::Select_coreContext* context) override {
        SelectStatement statement(StatementType::SELECT_STATEMENT);

        if (context->whereExpr) {
            std::any whereClause = visit(context->whereExpr);
        }
        return statement;
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

        if (ctx->literal_value()) {
            return visit(ctx->literal_value());
        }

        if (ctx->column_name()) {
            return visit(ctx->column_name());
        }


        return visitChildren(ctx);
    }

    std::any visitLiteral_value(SQLiteParser::Literal_valueContext* context) override {
        return visitChildren(context);
    }
};
