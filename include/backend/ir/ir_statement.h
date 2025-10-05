#ifndef IR_STATEMENT_H
#define IR_STATEMENT_H

#include "backend/ir/ir_types.h"
#include "backend/ir/ir_core.h"

// ============================================================================
// IR STATEMENT GENERATION - Statement-specific IR generation logic
// ============================================================================

// Statement generation
void ir_generate_statement_impl(IRFunction *ir_func, Stmt *stmt, SemanticAnalyzer *analyzer);

// Statement analysis utilities
bool stmt_always_returns(Stmt *stmt);

#endif // IR_STATEMENT_H
