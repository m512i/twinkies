#ifndef IR_EXPRESSION_H
#define IR_EXPRESSION_H

#include "backend/ir/ir_types.h"
#include "backend/ir/ir_core.h"

// ============================================================================
// IR EXPRESSION GENERATION - Expression-specific IR generation logic
// ============================================================================

// Expression generation
IROperand *ir_generate_expression_impl(IRFunction *ir_func, Expr *expr, SemanticAnalyzer *analyzer, DataType expected_type);

#endif // IR_EXPRESSION_H
