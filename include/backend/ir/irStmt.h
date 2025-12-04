#ifndef IR_STATEMENT_H
#define IR_STATEMENT_H

#include "backend/ir/irTypes.h"
#include "backend/ir/irCore.h"

void ir_generate_statement_impl(IRFunction *ir_func, Stmt *stmt, SemanticAnalyzer *analyzer);
bool stmt_always_returns(Stmt *stmt);

#endif
