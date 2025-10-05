
// Generated from SQLiteParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "SQLiteParserVisitor.h"


/**
 * This class provides an empty implementation of SQLiteParserVisitor, which can be
 * extended to create a visitor which only needs to handle a subset of the available methods.
 */
class  SQLiteParserBaseVisitor : public SQLiteParserVisitor {
public:

  virtual std::any visitParse(SQLiteParser::ParseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSql_stmt_list(SQLiteParser::Sql_stmt_listContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSql_stmt(SQLiteParser::Sql_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlter_table_stmt(SQLiteParser::Alter_table_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAnalyze_stmt(SQLiteParser::Analyze_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAttach_stmt(SQLiteParser::Attach_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBegin_stmt(SQLiteParser::Begin_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCommit_stmt(SQLiteParser::Commit_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRollback_stmt(SQLiteParser::Rollback_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSavepoint_stmt(SQLiteParser::Savepoint_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRelease_stmt(SQLiteParser::Release_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCreate_index_stmt(SQLiteParser::Create_index_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIndexed_column(SQLiteParser::Indexed_columnContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCreate_table_stmt(SQLiteParser::Create_table_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitColumn_def(SQLiteParser::Column_defContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitType_name(SQLiteParser::Type_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitColumn_constraint(SQLiteParser::Column_constraintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSigned_number(SQLiteParser::Signed_numberContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTable_constraint(SQLiteParser::Table_constraintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForeign_key_clause(SQLiteParser::Foreign_key_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitConflict_clause(SQLiteParser::Conflict_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCreate_trigger_stmt(SQLiteParser::Create_trigger_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCreate_view_stmt(SQLiteParser::Create_view_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCreate_virtual_table_stmt(SQLiteParser::Create_virtual_table_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWith_clause(SQLiteParser::With_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCte_table_name(SQLiteParser::Cte_table_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRecursive_cte(SQLiteParser::Recursive_cteContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCommon_table_expression(SQLiteParser::Common_table_expressionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDelete_stmt(SQLiteParser::Delete_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDelete_stmt_limited(SQLiteParser::Delete_stmt_limitedContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDetach_stmt(SQLiteParser::Detach_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDrop_stmt(SQLiteParser::Drop_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpr(SQLiteParser::ExprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRaise_function(SQLiteParser::Raise_functionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLiteral_value(SQLiteParser::Literal_valueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValue_row(SQLiteParser::Value_rowContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitValues_clause(SQLiteParser::Values_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInsert_stmt(SQLiteParser::Insert_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReturning_clause(SQLiteParser::Returning_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUpsert_clause(SQLiteParser::Upsert_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPragma_stmt(SQLiteParser::Pragma_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPragma_value(SQLiteParser::Pragma_valueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitReindex_stmt(SQLiteParser::Reindex_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelect_stmt(SQLiteParser::Select_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitJoin_clause(SQLiteParser::Join_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSelect_core(SQLiteParser::Select_coreContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFactored_select_stmt(SQLiteParser::Factored_select_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimple_select_stmt(SQLiteParser::Simple_select_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCompound_select_stmt(SQLiteParser::Compound_select_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTable_or_subquery(SQLiteParser::Table_or_subqueryContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitResult_column(SQLiteParser::Result_columnContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitJoin_operator(SQLiteParser::Join_operatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitJoin_constraint(SQLiteParser::Join_constraintContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCompound_operator(SQLiteParser::Compound_operatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUpdate_stmt(SQLiteParser::Update_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitColumn_name_list(SQLiteParser::Column_name_listContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUpdate_stmt_limited(SQLiteParser::Update_stmt_limitedContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitQualified_table_name(SQLiteParser::Qualified_table_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitVacuum_stmt(SQLiteParser::Vacuum_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFilter_clause(SQLiteParser::Filter_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWindow_defn(SQLiteParser::Window_defnContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOver_clause(SQLiteParser::Over_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFrame_spec(SQLiteParser::Frame_specContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFrame_clause(SQLiteParser::Frame_clauseContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimple_function_invocation(SQLiteParser::Simple_function_invocationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAggregate_function_invocation(SQLiteParser::Aggregate_function_invocationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWindow_function_invocation(SQLiteParser::Window_function_invocationContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCommon_table_stmt(SQLiteParser::Common_table_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrder_by_stmt(SQLiteParser::Order_by_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitLimit_stmt(SQLiteParser::Limit_stmtContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrdering_term(SQLiteParser::Ordering_termContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAsc_desc(SQLiteParser::Asc_descContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFrame_left(SQLiteParser::Frame_leftContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFrame_right(SQLiteParser::Frame_rightContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFrame_single(SQLiteParser::Frame_singleContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWindow_function(SQLiteParser::Window_functionContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOffset(SQLiteParser::OffsetContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitDefault_value(SQLiteParser::Default_valueContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPartition_by(SQLiteParser::Partition_byContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrder_by_expr(SQLiteParser::Order_by_exprContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitOrder_by_expr_asc_desc(SQLiteParser::Order_by_expr_asc_descContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitExpr_asc_desc(SQLiteParser::Expr_asc_descContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitInitial_select(SQLiteParser::Initial_selectContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitRecursive_select(SQLiteParser::Recursive_selectContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitUnary_operator(SQLiteParser::Unary_operatorContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitError_message(SQLiteParser::Error_messageContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitModule_argument(SQLiteParser::Module_argumentContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitColumn_alias(SQLiteParser::Column_aliasContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitKeyword(SQLiteParser::KeywordContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitName(SQLiteParser::NameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFunction_name(SQLiteParser::Function_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSchema_name(SQLiteParser::Schema_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTable_name(SQLiteParser::Table_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTable_or_index_name(SQLiteParser::Table_or_index_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitColumn_name(SQLiteParser::Column_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitCollation_name(SQLiteParser::Collation_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitForeign_table(SQLiteParser::Foreign_tableContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitIndex_name(SQLiteParser::Index_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTrigger_name(SQLiteParser::Trigger_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitView_name(SQLiteParser::View_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitModule_name(SQLiteParser::Module_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitPragma_name(SQLiteParser::Pragma_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSavepoint_name(SQLiteParser::Savepoint_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTable_alias(SQLiteParser::Table_aliasContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTransaction_name(SQLiteParser::Transaction_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitWindow_name(SQLiteParser::Window_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAlias(SQLiteParser::AliasContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitFilename(SQLiteParser::FilenameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitBase_window_name(SQLiteParser::Base_window_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitSimple_func(SQLiteParser::Simple_funcContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAggregate_func(SQLiteParser::Aggregate_funcContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitTable_function_name(SQLiteParser::Table_function_nameContext *ctx) override {
    return visitChildren(ctx);
  }

  virtual std::any visitAny_name(SQLiteParser::Any_nameContext *ctx) override {
    return visitChildren(ctx);
  }


};

