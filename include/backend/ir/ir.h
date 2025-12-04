#ifndef IR_H
#define IR_H

#include "backend/ir/irTypes.h"
#include "backend/ir/irCore.h"
#include "backend/ir/irGenerator.h"
#include "backend/ir/irStmt.h"
#include "backend/ir/irExpr.h"
#include "backend/ir/irOps.h"
#include "backend/ir/irinstructions.h"

IRProgram *ir_generate(Program *ast_program, SemanticAnalyzer *analyzer);
IRProgram *ir_generate_with_modules(Program *ast_program, SemanticAnalyzer *analyzer, void *module_manager);
IRFunction *ir_generate_function(Function *func, SemanticAnalyzer *analyzer);
void ir_generate_statement(IRFunction *ir_func, Stmt *stmt, SemanticAnalyzer *analyzer);
IROperand *ir_generate_expression(IRFunction *ir_func, Expr *expr, SemanticAnalyzer *analyzer, DataType expected_type);

#endif