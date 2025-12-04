#include "backend/ir/ir.h"
#include "analysis/semantic/semantic.h"
#include "backend/ir/irOps.h"
#include "backend/ir/irinstructions.h"
#include "common/common.h"
#include "modules/modules.h"

extern bool debug_enabled;

IRProgram *ir_generate(Program *ast_program, SemanticAnalyzer *analyzer)
{
    extern IRProgram *ir_generate_impl(Program *ast_program, SemanticAnalyzer *analyzer);
    return ir_generate_impl(ast_program, analyzer);
}

IRProgram *ir_generate_with_modules(Program *ast_program, SemanticAnalyzer *analyzer, void *module_manager)
{
    extern IRProgram *ir_generate_with_modules_impl(Program *ast_program, SemanticAnalyzer *analyzer, void *module_manager);
    return ir_generate_with_modules_impl(ast_program, analyzer, module_manager);
}

IRFunction *ir_generate_function(Function *func, SemanticAnalyzer *analyzer)
{
    extern IRFunction *ir_generate_function_impl(Function *func, SemanticAnalyzer *analyzer);
    return ir_generate_function_impl(func, analyzer);
}

void ir_generate_statement(IRFunction *ir_func, Stmt *stmt, SemanticAnalyzer *analyzer)
{
    extern void ir_generate_statement_impl(IRFunction *ir_func, Stmt *stmt, SemanticAnalyzer *analyzer);
    ir_generate_statement_impl(ir_func, stmt, analyzer);
}

IROperand *ir_generate_expression(IRFunction *ir_func, Expr *expr, SemanticAnalyzer *analyzer, DataType expected_type)
{
    extern IROperand *ir_generate_expression_impl(IRFunction *ir_func, Expr *expr, SemanticAnalyzer *analyzer, DataType expected_type);
    return ir_generate_expression_impl(ir_func, expr, analyzer, expected_type);
}