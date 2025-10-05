#ifndef IR_H
#define IR_H

#include "backend/ir/ir_types.h"
#include "backend/ir/ir_core.h"
#include "backend/ir/ir_generator.h"
#include "backend/ir/ir_statement.h"
#include "backend/ir/ir_expression.h"
#include "backend/ir/iroperands.h"
#include "backend/ir/irinstructions.h"

// Public API entry points (declarations for functions implemented in other modules)
IRProgram *ir_generate(Program *ast_program, SemanticAnalyzer *analyzer);
IRProgram *ir_generate_with_modules(Program *ast_program, SemanticAnalyzer *analyzer, void *module_manager);
IRFunction *ir_generate_function(Function *func, SemanticAnalyzer *analyzer);
void ir_generate_statement(IRFunction *ir_func, Stmt *stmt, SemanticAnalyzer *analyzer);
IROperand *ir_generate_expression(IRFunction *ir_func, Expr *expr, SemanticAnalyzer *analyzer, DataType expected_type);

#endif