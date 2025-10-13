#pragma once

#include "SQLiteParserBaseVisitor.h"
#include <iostream>

#include "parser/CreateTableStatement.h"
#include "parser/SelectStatement.h"


class SqliteAstBuilder : public SQLiteParserBaseVisitor {
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


    std::any visitAndExpr(SQLiteParser::AndExprContext* context) override {
        std::cerr << context->expr().size() << std::endl;

        auto left = std::any_cast<std::shared_ptr<ParsedExpression>>(visit(context->expr()[0]));

        auto right = std::any_cast<std::shared_ptr<ParsedExpression>>(visit(context->expr()[1]));


        auto expression = std::make_shared<ParsedExpression>("AND");
        expression->left = std::move(left);
        expression->right = std::move(right);

        return expression;
    }

    std::any visitEqualityExpr(SQLiteParser::EqualityExprContext* context) override {
        std::any data = visit(context->expr()[0]);
        auto left = std::any_cast<std::shared_ptr<ParsedExpression>>(data);

        auto right = std::any_cast<std::shared_ptr<ParsedExpression>>(visit(context->expr()[1]));

        auto expression = std::make_shared<ParsedExpression>("ASSIGN");
        expression->left = std::move(left);
        expression->right = std::move(right);
        return expression;
    }

    std::any visitColumn_name(SQLiteParser::Column_nameContext* context) override {
        auto expression = std::make_shared<ParsedExpression>("COLUMN");
        expression->value = context->getText();
        return expression;
    }

    std::any visitLiteralExpr(SQLiteParser::LiteralExprContext* context) override {
        auto expression = std::make_shared<ParsedExpression>("LITERAL");
        expression->value = context->getText();
        return expression;
    }

    std::any visitQualifiedColunExpr(SQLiteParser::QualifiedColunExprContext* context) override {
        auto expression = std::make_shared<ParsedExpression>("QualifiedColumn");
        expression->value = context->getText();
        return expression;
    }

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
        if (statement.type() == typeid(std::shared_ptr<SelectStatement>)) {
            auto stt = std::any_cast<std::shared_ptr<SelectStatement>>(statement);
            return stt;
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

        if (statement.type() == typeid(std::shared_ptr<SelectStatement>)) {
            auto stt = std::any_cast<std::shared_ptr<SelectStatement>>(statement);
            return stt;
        }
        throw std::runtime_error(" visit stmt list Parsing failed only create table statement is implemented");
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

    std::any visitResult_column(SQLiteParser::Result_columnContext* context) override {
        auto columnExpr = std::any_cast<std::shared_ptr<ParsedExpression>>(visit(context->expr()));

        return columnExpr.get()->value;
    }

    std::any visitSelect_core(SQLiteParser::Select_coreContext* context) override {
        auto statement = std::make_shared<SelectStatement>(StatementType::SELECT_STATEMENT);
        if (context->whereExpr) {
            std::any whereClause = visit(context->whereExpr);
            statement->whereClause = std::move(std::any_cast<std::shared_ptr<ParsedExpression>>(whereClause));
        }

        if (!context->table_or_subquery().empty() && context->table_or_subquery().size() == 1) {
            statement->fromTable = std::any_cast<std::shared_ptr<Table>>(visit(context->table_or_subquery()[0]));
        }

        std::vector<Column> columns;

        for (auto col : context->result_column()) {
            std::string name = std::any_cast<std::string>(visit(col));
            Column c;
            c.name = name;
            columns.emplace_back(c);
        }

        statement->fromTable->columns = columns;

        return statement;
    }


    std::any visitTableAliasIndex(SQLiteParser::TableAliasIndexContext* context) override {
        auto tableName = context->table_name()->getText();

        auto table = std::make_shared<Table>(context->table_name()->getText());

        return table;
    }


    std::any visitColumn_constraint(SQLiteParser::Column_constraintContext* ctx) override {
        auto constraint = std::make_shared<Constraint>(ConstraintType::COLUMN_CONSTRAINT);
        constraint->constraintType = ConstraintType::COLUMN_CONSTRAINT;
        if (ctx->AUTOINCREMENT_()) {
            constraint->autoIncrement = true;
        }
        return constraint;
    }

    std::any visitColumn_def(SQLiteParser::Column_defContext* ctx) override {
        auto definition = std::make_shared<ColumnDefinition>(ctx->column_name()->getText(), dataTypeFromString(ctx->type_name()->getText()));
        for (auto constraint : ctx->column_constraint()) {
            definition->constraints.emplace_back(std::any_cast<std::shared_ptr<Constraint>>(visit(constraint)));
        }
        return definition;
    }

    std::any visitTable_constraint(SQLiteParser::Table_constraintContext* context) override {
        auto constraint = std::make_shared<Constraint>(ConstraintType::TABLE_CONSTRAINT);
        if (context->PRIMARY_()) {
            constraint->constraintType = ConstraintType::TABLE_CONSTRAINT;
            constraint->primary = true;
        }
        if (context->UNIQUE_()) {
            constraint->unique = true;
        }
        return constraint;
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

        std::vector<std::shared_ptr<ColumnDefinition>> columns;
        for (int i = 0; i < ctx.size(); i++) {
            columns.emplace_back(std::any_cast<std::shared_ptr<ColumnDefinition>>(visit(ctx[i])));
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

    std::any visitFunctionCallExpr(SQLiteParser::FunctionCallExprContext* context) override {
        std::cerr << context->function_name()->getText() << std::endl;

        return visitChildren(context);
    }

    /*std::any visitExpr(SQLiteParser::ExprContext* ctx) override {
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
    }*/

    std::any visitLiteral_value(SQLiteParser::Literal_valueContext* context) override {
        return visitChildren(context);
    }
};
