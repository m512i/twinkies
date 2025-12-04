#ifndef IR_EXPRESSION_H
#define IR_EXPRESSION_H

#include "backend/ir/irTypes.h"
#include "backend/ir/irCore.h"

IROperand *ir_generate_expression_impl(IRFunction *ir_func, Expr *expr, SemanticAnalyzer *analyzer, DataType expected_type);

#endif
