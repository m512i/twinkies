#include "backend/codegen_strategy.h"
#include "backend/codegen_c_writer.h"
#include "backend/codegen_ffi.h"
#include "backend/codegen_instruction_handlers.h"
#include <stdlib.h>
#include <string.h>

static void c_strategy_generate_header(CodeGenerator *generator) {
    codegen_c_writer_write_header(generator);
}

static void c_strategy_generate_program(CodeGenerator *generator) {
    codegen_core_generate_program(generator);
}

static void c_strategy_generate_function(CodeGenerator *generator, IRFunction *func) {
    codegen_c_writer_write_function_header(generator, func);

    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (instr->opcode == IR_ARRAY_DECL || instr->opcode == IR_VAR_DECL)
            continue;
        codegen_instruction_handlers_generate_instruction(generator, instr);
    }

    codegen_c_writer_write_function_footer(generator);
}

static void c_strategy_generate_instruction(CodeGenerator *generator, IRInstruction *instr) {
    codegen_instruction_handlers_generate_instruction(generator, instr);
}

static void c_strategy_write_operand(CodeGenerator *generator, IROperand *operand) {
    codegen_c_writer_write_operand(generator, operand);
}

static void c_strategy_write_ffi_declarations(CodeGenerator *generator, Program *program) {
    codegen_ffi_write_declarations(generator, program);
}

static void c_strategy_write_ffi_loading(CodeGenerator *generator, Program *program) {
    codegen_ffi_write_loading(generator, program);
}

static void c_strategy_write_runtime_functions(CodeGenerator *generator) {
    codegen_c_writer_write_runtime_functions(generator);
}

static void c_strategy_destroy(CodeGenStrategy *strategy) {
    safe_free(strategy);
}

CodeGenStrategy *c_codegen_strategy_create(void) {
    CodeGenStrategy *strategy = safe_malloc(sizeof(CodeGenStrategy));
    strategy->generate_header = c_strategy_generate_header;
    strategy->generate_program = c_strategy_generate_program;
    strategy->generate_function = c_strategy_generate_function;
    strategy->generate_instruction = c_strategy_generate_instruction;
    strategy->write_operand = c_strategy_write_operand;
    strategy->write_ffi_declarations = c_strategy_write_ffi_declarations;
    strategy->write_ffi_loading = c_strategy_write_ffi_loading;
    strategy->write_runtime_functions = c_strategy_write_runtime_functions;
    strategy->destroy = c_strategy_destroy;
    return strategy;
}

CodeGenStrategy *asm_codegen_strategy_create(void) {
    // Implement assembly code generation strategy
    return NULL;
}

CodeGenStrategy *codegen_strategy_factory(const char *target_language) {
    if (!target_language) return NULL;
    
    if (strcmp(target_language, "c") == 0) {
        return c_codegen_strategy_create();
    } else if (strcmp(target_language, "asm") == 0) {
        return asm_codegen_strategy_create();
    }
    
    return NULL;
}
