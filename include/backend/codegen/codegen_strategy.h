#ifndef CODEGEN_STRATEGY_H
#define CODEGEN_STRATEGY_H

#include "common/common.h"
#include "backend/ir/ir.h"
#include "frontend/ast/ast.h"
#include "backend/codegen/codegen_core.h"
#include <stdio.h>

typedef struct CodeGenStrategy {
    void (*generate_header)(CodeGenerator *generator);
    void (*generate_program)(CodeGenerator *generator);
    void (*generate_function)(CodeGenerator *generator, IRFunction *func);
    void (*generate_instruction)(CodeGenerator *generator, IRInstruction *instr);
    void (*write_operand)(CodeGenerator *generator, IROperand *operand);
    void (*write_ffi_declarations)(CodeGenerator *generator, Program *program);
    void (*write_ffi_loading)(CodeGenerator *generator, Program *program);
    
    void (*write_runtime_functions)(CodeGenerator *generator);
    
    void (*destroy)(struct CodeGenStrategy *strategy);
} CodeGenStrategy;

CodeGenStrategy *codegen_strategy_factory(const char *target_language);

CodeGenStrategy *c_codegen_strategy_create(void);

CodeGenStrategy *asm_codegen_strategy_create(void);

#endif 