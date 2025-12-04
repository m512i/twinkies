#ifndef IR_GENERATOR_H
#define IR_GENERATOR_H

#include "backend/ir/irTypes.h"
#include "backend/ir/irCore.h"
#include "modules/modules.h"

IRProgram *ir_generate_impl(Program *ast_program, SemanticAnalyzer *analyzer);
IRProgram *ir_generate_with_modules_impl(Program *ast_program, SemanticAnalyzer *analyzer, void *module_manager);
IRFunction *ir_generate_function_impl(Function *func, SemanticAnalyzer *analyzer);

#endif
