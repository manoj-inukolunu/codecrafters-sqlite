
// Generated from SQLiteParser.g4 by ANTLR 4.13.2

#pragma once


#include "antlr4-runtime.h"
#include "SQLiteParser.h"



/**
 * This class defines an abstract visitor for a parse tree
 * produced by SQLiteParser.
 */
class  SQLiteParserVisitor : public antlr4::tree::AbstractParseTreeVisitor {
public:

  /**
   * Visit parse trees produced by SQLiteParser.
   */
    virtual std::any visitParse(SQLiteParser::ParseContext *context) = 0;

    virtual std::any visitSql_stmt_list(SQLiteParser::Sql_stmt_listContext *context) = 0;

    virtual std::any visitSql_stmt(SQLiteParser::Sql_stmtContext *context) = 0;

    virtual std::any visitAlter_table_stmt(SQLiteParser::Alter_table_stmtContext *context) = 0;

    virtual std::any visitAnalyze_stmt(SQLiteParser::Analyze_stmtContext *context) = 0;

    virtual std::any visitAttach_stmt(SQLiteParser::Attach_stmtContext *context) = 0;

    virtual std::any visitBegin_stmt(SQLiteParser::Begin_stmtContext *context) = 0;

    virtual std::any visitCommit_stmt(SQLiteParser::Commit_stmtContext *context) = 0;

    virtual std::any visitRollback_stmt(SQLiteParser::Rollback_stmtContext *context) = 0;

    virtual std::any visitSavepoint_stmt(SQLiteParser::Savepoint_stmtContext *context) = 0;

    virtual std::any visitRelease_stmt(SQLiteParser::Release_stmtContext *context) = 0;

    virtual std::any visitCreate_index_stmt(SQLiteParser::Create_index_stmtContext *context) = 0;

    virtual std::any visitIndexed_column(SQLiteParser::Indexed_columnContext *context) = 0;

    virtual std::any visitCreate_table_stmt(SQLiteParser::Create_table_stmtContext *context) = 0;

    virtual std::any visitColumn_def(SQLiteParser::Column_defContext *context) = 0;

    virtual std::any visitType_name(SQLiteParser::Type_nameContext *context) = 0;

    virtual std::any visitColumn_constraint(SQLiteParser::Column_constraintContext *context) = 0;

    virtual std::any visitSigned_number(SQLiteParser::Signed_numberContext *context) = 0;

    virtual std::any visitTable_constraint(SQLiteParser::Table_constraintContext *context) = 0;

    virtual std::any visitForeign_key_clause(SQLiteParser::Foreign_key_clauseContext *context) = 0;

    virtual std::any visitConflict_clause(SQLiteParser::Conflict_clauseContext *context) = 0;

    virtual std::any visitCreate_trigger_stmt(SQLiteParser::Create_trigger_stmtContext *context) = 0;

    virtual std::any visitCreate_view_stmt(SQLiteParser::Create_view_stmtContext *context) = 0;

    virtual std::any visitCreate_virtual_table_stmt(SQLiteParser::Create_virtual_table_stmtContext *context) = 0;

    virtual std::any visitWith_clause(SQLiteParser::With_clauseContext *context) = 0;

    virtual std::any visitCte_table_name(SQLiteParser::Cte_table_nameContext *context) = 0;

    virtual std::any visitRecursive_cte(SQLiteParser::Recursive_cteContext *context) = 0;

    virtual std::any visitCommon_table_expression(SQLiteParser::Common_table_expressionContext *context) = 0;

    virtual std::any visitDelete_stmt(SQLiteParser::Delete_stmtContext *context) = 0;

    virtual std::any visitDelete_stmt_limited(SQLiteParser::Delete_stmt_limitedContext *context) = 0;

    virtual std::any visitDetach_stmt(SQLiteParser::Detach_stmtContext *context) = 0;

    virtual std::any visitDrop_stmt(SQLiteParser::Drop_stmtContext *context) = 0;

    virtual std::any visitQualifiedColunExpr(SQLiteParser::QualifiedColunExprContext *context) = 0;

    virtual std::any visitAddSubExpr(SQLiteParser::AddSubExprContext *context) = 0;

    virtual std::any visitCastExpr(SQLiteParser::CastExprContext *context) = 0;

    virtual std::any visitRaiseExpr(SQLiteParser::RaiseExprContext *context) = 0;

    virtual std::any visitInExpr(SQLiteParser::InExprContext *context) = 0;

    virtual std::any visitOrExpr(SQLiteParser::OrExprContext *context) = 0;

    virtual std::any visitMulDivModExpr(SQLiteParser::MulDivModExprContext *context) = 0;

    virtual std::any visitExistsExpr(SQLiteParser::ExistsExprContext *context) = 0;

    virtual std::any visitParenExpr(SQLiteParser::ParenExprContext *context) = 0;

    virtual std::any visitBetweenExpr(SQLiteParser::BetweenExprContext *context) = 0;

    virtual std::any visitBitwiseExpr(SQLiteParser::BitwiseExprContext *context) = 0;

    virtual std::any visitUnaryExpr(SQLiteParser::UnaryExprContext *context) = 0;

    virtual std::any visitLiteralExpr(SQLiteParser::LiteralExprContext *context) = 0;

    virtual std::any visitNullCheckExpr(SQLiteParser::NullCheckExprContext *context) = 0;

    virtual std::any visitFunctionCallExpr(SQLiteParser::FunctionCallExprContext *context) = 0;

    virtual std::any visitPipe2Expr(SQLiteParser::Pipe2ExprContext *context) = 0;

    virtual std::any visitLikeExpr(SQLiteParser::LikeExprContext *context) = 0;

    virtual std::any visitBindParamExpr(SQLiteParser::BindParamExprContext *context) = 0;

    virtual std::any visitCaseExpr(SQLiteParser::CaseExprContext *context) = 0;

    virtual std::any visitIsExpr(SQLiteParser::IsExprContext *context) = 0;

    virtual std::any visitEqualityExpr(SQLiteParser::EqualityExprContext *context) = 0;

    virtual std::any visitCollateExpr(SQLiteParser::CollateExprContext *context) = 0;

    virtual std::any visitCompareExpr(SQLiteParser::CompareExprContext *context) = 0;

    virtual std::any visitAndExpr(SQLiteParser::AndExprContext *context) = 0;

    virtual std::any visitRaise_function(SQLiteParser::Raise_functionContext *context) = 0;

    virtual std::any visitLiteral_value(SQLiteParser::Literal_valueContext *context) = 0;

    virtual std::any visitValue_row(SQLiteParser::Value_rowContext *context) = 0;

    virtual std::any visitValues_clause(SQLiteParser::Values_clauseContext *context) = 0;

    virtual std::any visitInsert_stmt(SQLiteParser::Insert_stmtContext *context) = 0;

    virtual std::any visitReturning_clause(SQLiteParser::Returning_clauseContext *context) = 0;

    virtual std::any visitUpsert_clause(SQLiteParser::Upsert_clauseContext *context) = 0;

    virtual std::any visitPragma_stmt(SQLiteParser::Pragma_stmtContext *context) = 0;

    virtual std::any visitPragma_value(SQLiteParser::Pragma_valueContext *context) = 0;

    virtual std::any visitReindex_stmt(SQLiteParser::Reindex_stmtContext *context) = 0;

    virtual std::any visitSelect_stmt(SQLiteParser::Select_stmtContext *context) = 0;

    virtual std::any visitJoin_clause(SQLiteParser::Join_clauseContext *context) = 0;

    virtual std::any visitSelect_core(SQLiteParser::Select_coreContext *context) = 0;

    virtual std::any visitFactored_select_stmt(SQLiteParser::Factored_select_stmtContext *context) = 0;

    virtual std::any visitSimple_select_stmt(SQLiteParser::Simple_select_stmtContext *context) = 0;

    virtual std::any visitCompound_select_stmt(SQLiteParser::Compound_select_stmtContext *context) = 0;

    virtual std::any visitTableAliasIndex(SQLiteParser::TableAliasIndexContext *context) = 0;

    virtual std::any visitTableFunction(SQLiteParser::TableFunctionContext *context) = 0;

    virtual std::any visitTableOrJoin(SQLiteParser::TableOrJoinContext *context) = 0;

    virtual std::any visitSubquery(SQLiteParser::SubqueryContext *context) = 0;

    virtual std::any visitResult_column(SQLiteParser::Result_columnContext *context) = 0;

    virtual std::any visitJoin_operator(SQLiteParser::Join_operatorContext *context) = 0;

    virtual std::any visitJoin_constraint(SQLiteParser::Join_constraintContext *context) = 0;

    virtual std::any visitCompound_operator(SQLiteParser::Compound_operatorContext *context) = 0;

    virtual std::any visitUpdate_stmt(SQLiteParser::Update_stmtContext *context) = 0;

    virtual std::any visitColumn_name_list(SQLiteParser::Column_name_listContext *context) = 0;

    virtual std::any visitUpdate_stmt_limited(SQLiteParser::Update_stmt_limitedContext *context) = 0;

    virtual std::any visitQualified_table_name(SQLiteParser::Qualified_table_nameContext *context) = 0;

    virtual std::any visitVacuum_stmt(SQLiteParser::Vacuum_stmtContext *context) = 0;

    virtual std::any visitFilter_clause(SQLiteParser::Filter_clauseContext *context) = 0;

    virtual std::any visitWindow_defn(SQLiteParser::Window_defnContext *context) = 0;

    virtual std::any visitOver_clause(SQLiteParser::Over_clauseContext *context) = 0;

    virtual std::any visitFrame_spec(SQLiteParser::Frame_specContext *context) = 0;

    virtual std::any visitFrame_clause(SQLiteParser::Frame_clauseContext *context) = 0;

    virtual std::any visitSimple_function_invocation(SQLiteParser::Simple_function_invocationContext *context) = 0;

    virtual std::any visitAggregate_function_invocation(SQLiteParser::Aggregate_function_invocationContext *context) = 0;

    virtual std::any visitWindow_function_invocation(SQLiteParser::Window_function_invocationContext *context) = 0;

    virtual std::any visitCommon_table_stmt(SQLiteParser::Common_table_stmtContext *context) = 0;

    virtual std::any visitOrder_by_stmt(SQLiteParser::Order_by_stmtContext *context) = 0;

    virtual std::any visitLimit_stmt(SQLiteParser::Limit_stmtContext *context) = 0;

    virtual std::any visitOrdering_term(SQLiteParser::Ordering_termContext *context) = 0;

    virtual std::any visitAsc_desc(SQLiteParser::Asc_descContext *context) = 0;

    virtual std::any visitFrame_left(SQLiteParser::Frame_leftContext *context) = 0;

    virtual std::any visitFrame_right(SQLiteParser::Frame_rightContext *context) = 0;

    virtual std::any visitFrame_single(SQLiteParser::Frame_singleContext *context) = 0;

    virtual std::any visitWindow_function(SQLiteParser::Window_functionContext *context) = 0;

    virtual std::any visitOffset(SQLiteParser::OffsetContext *context) = 0;

    virtual std::any visitDefault_value(SQLiteParser::Default_valueContext *context) = 0;

    virtual std::any visitPartition_by(SQLiteParser::Partition_byContext *context) = 0;

    virtual std::any visitOrder_by_expr(SQLiteParser::Order_by_exprContext *context) = 0;

    virtual std::any visitOrder_by_expr_asc_desc(SQLiteParser::Order_by_expr_asc_descContext *context) = 0;

    virtual std::any visitExpr_asc_desc(SQLiteParser::Expr_asc_descContext *context) = 0;

    virtual std::any visitInitial_select(SQLiteParser::Initial_selectContext *context) = 0;

    virtual std::any visitRecursive_select(SQLiteParser::Recursive_selectContext *context) = 0;

    virtual std::any visitUnary_operator(SQLiteParser::Unary_operatorContext *context) = 0;

    virtual std::any visitError_message(SQLiteParser::Error_messageContext *context) = 0;

    virtual std::any visitModule_argument(SQLiteParser::Module_argumentContext *context) = 0;

    virtual std::any visitColumn_alias(SQLiteParser::Column_aliasContext *context) = 0;

    virtual std::any visitKeyword(SQLiteParser::KeywordContext *context) = 0;

    virtual std::any visitName(SQLiteParser::NameContext *context) = 0;

    virtual std::any visitFunction_name(SQLiteParser::Function_nameContext *context) = 0;

    virtual std::any visitSchema_name(SQLiteParser::Schema_nameContext *context) = 0;

    virtual std::any visitTable_name(SQLiteParser::Table_nameContext *context) = 0;

    virtual std::any visitTable_or_index_name(SQLiteParser::Table_or_index_nameContext *context) = 0;

    virtual std::any visitColumn_name(SQLiteParser::Column_nameContext *context) = 0;

    virtual std::any visitCollation_name(SQLiteParser::Collation_nameContext *context) = 0;

    virtual std::any visitForeign_table(SQLiteParser::Foreign_tableContext *context) = 0;

    virtual std::any visitIndex_name(SQLiteParser::Index_nameContext *context) = 0;

    virtual std::any visitTrigger_name(SQLiteParser::Trigger_nameContext *context) = 0;

    virtual std::any visitView_name(SQLiteParser::View_nameContext *context) = 0;

    virtual std::any visitModule_name(SQLiteParser::Module_nameContext *context) = 0;

    virtual std::any visitPragma_name(SQLiteParser::Pragma_nameContext *context) = 0;

    virtual std::any visitSavepoint_name(SQLiteParser::Savepoint_nameContext *context) = 0;

    virtual std::any visitTable_alias(SQLiteParser::Table_aliasContext *context) = 0;

    virtual std::any visitTransaction_name(SQLiteParser::Transaction_nameContext *context) = 0;

    virtual std::any visitWindow_name(SQLiteParser::Window_nameContext *context) = 0;

    virtual std::any visitAlias(SQLiteParser::AliasContext *context) = 0;

    virtual std::any visitFilename(SQLiteParser::FilenameContext *context) = 0;

    virtual std::any visitBase_window_name(SQLiteParser::Base_window_nameContext *context) = 0;

    virtual std::any visitSimple_func(SQLiteParser::Simple_funcContext *context) = 0;

    virtual std::any visitAggregate_func(SQLiteParser::Aggregate_funcContext *context) = 0;

    virtual std::any visitTable_function_name(SQLiteParser::Table_function_nameContext *context) = 0;

    virtual std::any visitAny_name(SQLiteParser::Any_nameContext *context) = 0;


};

