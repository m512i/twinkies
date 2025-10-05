#ifndef IR_CORE_H
#define IR_CORE_H

#include "backend/ir/ir_types.h"
#include "backend/ir/iroperands.h"
#include "backend/ir/irinstructions.h"

// ============================================================================
// IR CORE OPERATIONS - Basic IR data structures and operations
// ============================================================================

// Loop context management
LoopContext *ir_loop_context_create(char *start_label, char *end_label);
void ir_loop_context_destroy(LoopContext *context);
void ir_function_enter_loop(IRFunction *func, char *start_label, char *end_label);
void ir_function_exit_loop(IRFunction *func);
LoopContext *ir_function_get_current_loop(IRFunction *func);

// IR data structure creation and management
IRFunction *ir_function_create(const char *name, DataType return_type);
IRProgram *ir_program_create(void);
void ir_function_destroy(IRFunction *func);
void ir_program_destroy(IRProgram *program);

// IR instruction and parameter management
void ir_function_add_instruction(IRFunction *func, IRInstruction *instr);
void ir_function_add_param(IRFunction *func, IROperand *param);
void ir_program_add_function(IRProgram *program, IRFunction *func);

// IR printing and debugging
void ir_function_print(const IRFunction *func);
void ir_program_print(const IRProgram *program);

// IR utility functions
const char *ir_opcode_to_string(IROpcode opcode);
int ir_function_new_temp(IRFunction *func);
char *ir_function_new_label(IRFunction *func);

#endif // IR_CORE_H
