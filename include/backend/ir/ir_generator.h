#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include "backend/ir/ir_types.h"
#include "backend/ir/ir_core.h"
#include "modules/modules.h"

// ============================================================================
// IR GENERATION STRATEGY - Main IR generation orchestration
// ============================================================================

// Main IR generation entry points
IRProgram *ir_generate_impl(Program *ast_program, SemanticAnalyzer *analyzer);
IRProgram *ir_generate_with_modules_impl(Program *ast_program, SemanticAnalyzer *analyzer, void *module_manager);
IRFunction *ir_generate_function_impl(Function *func, SemanticAnalyzer *analyzer);

#endif // IR_GENERATOR_H
