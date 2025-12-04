#ifndef CODEGEN_INSTRUCTION_HANDLERS_H
#define CODEGEN_INSTRUCTION_HANDLERS_H

#include "common/common.h"
#include "backend/ir/ir.h"
#include "backend/codegen/codegenCore.h"
#include <stdio.h>
void codegen_handle_nop(CodeGenerator *generator, IRInstruction *instr);
void codegen_handle_label(CodeGenerator *generator, IRInstruction *instr);
void codegen_handle_jump(CodeGenerator *generator, IRInstruction *instr);
void codegen_handle_jump_if(CodeGenerator *generator, IRInstruction *instr);
void codegen_handle_jump_if_false(CodeGenerator *generator, IRInstruction *instr);
void codegen_handle_return(CodeGenerator *generator, IRInstruction *instr);
void codegen_handle_move(CodeGenerator *generator, IRInstruction *instr);
void codegen_handle_arithmetic(CodeGenerator *generator, IRInstruction *instr);
void codegen_handle_unary_arithmetic(CodeGenerator *generator, IRInstruction *instr);

void codegen_handle_comparison(CodeGenerator *generator, IRInstruction *instr);

void codegen_handle_param(CodeGenerator *generator, IRInstruction *instr);
void codegen_handle_call(CodeGenerator *generator, IRInstruction *instr);
void codegen_handle_print(CodeGenerator *generator, IRInstruction *instr);

void codegen_handle_array_load(CodeGenerator *generator, IRInstruction *instr);
void codegen_handle_array_store(CodeGenerator *generator, IRInstruction *instr);
void codegen_handle_bounds_check(CodeGenerator *generator, IRInstruction *instr);
void codegen_handle_array_decl(CodeGenerator *generator, IRInstruction *instr);
void codegen_handle_array_init(CodeGenerator *generator, IRInstruction *instr);
void codegen_handle_var_decl(CodeGenerator *generator, IRInstruction *instr);
void codegen_handle_inline_asm(CodeGenerator *generator, IRInstruction *instr);

void codegen_instruction_handlers_generate_instruction(CodeGenerator *generator, IRInstruction *instr);

#endif