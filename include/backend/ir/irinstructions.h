#ifndef IRINSTRUCTIONS_H
#define IRINSTRUCTIONS_H

#include "backend/ir/irTypes.h"


IRInstruction *ir_instruction_nop(void);
IRInstruction *ir_instruction_label(const char *label);
IRInstruction *ir_instruction_move(IROperand *result, IROperand *source);
IRInstruction *ir_instruction_binary(IROpcode opcode, IROperand *result, IROperand *arg1, IROperand *arg2);
IRInstruction *ir_instruction_unary(IROpcode opcode, IROperand *result, IROperand *arg);
IRInstruction *ir_instruction_jump(const char *label);
IRInstruction *ir_instruction_jump_if(IROperand *condition, const char *label);
IRInstruction *ir_instruction_jump_if_false(IROperand *condition, const char *label);
IRInstruction *ir_instruction_call(IROperand *result, const char *func_name);
IRInstruction *ir_instruction_return(IROperand *value);
IRInstruction *ir_instruction_param(IROperand *param);
IRInstruction *ir_instruction_print_op(IROperand *value);
IRInstruction *ir_instruction_print_multiple(DynamicArray *args);
IRInstruction *ir_instruction_array_load(IROperand *result, IROperand *array, IROperand *index);
IRInstruction *ir_instruction_array_store(IROperand *array, IROperand *index, IROperand *value);
IRInstruction *ir_instruction_bounds_check(IROperand *index, IROperand *size, const char *error_label);
IRInstruction *ir_instruction_array_decl(const char *array_name, int size, DataType element_type);
IRInstruction *ir_instruction_array_init(const char *array_name, int size, DataType element_type, IROperand *value);
IRInstruction *ir_instruction_var_decl(const char *var_name, DataType type);
IRInstruction *ir_instruction_inline_asm(const char *asm_code, bool is_volatile, DynamicArray *outputs, DynamicArray *inputs, DynamicArray *clobbers);

void ir_instruction_destroy(IRInstruction *instr);
void ir_instruction_print(const IRInstruction *instr);

#endif