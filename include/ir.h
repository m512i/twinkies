#ifndef IR_H
#define IR_H

#include "common.h"
#include "ast.h"

typedef enum {
    IR_NOP,
    IR_LABEL,
    IR_MOVE,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_MOD,
    IR_NEG,
    IR_NOT,
    IR_EQ,
    IR_NE,
    IR_LT,
    IR_LE,
    IR_GT,
    IR_GE,
    IR_AND,
    IR_OR,
    IR_JUMP,
    IR_JUMP_IF,
    IR_JUMP_IF_FALSE,
    IR_CALL,
    IR_RETURN,
    IR_PARAM,
    IR_PRINT
} IROpcode;

typedef enum {
    IR_OP_TEMP,    
    IR_OP_VAR,     
    IR_OP_CONST,   
    IR_OP_LABEL    
} IROperandType;

typedef struct {
    IROperandType type;
    union {
        int temp_id;
        char* var_name;
        int64_t const_value;
        char* label_name;
    } data;
} IROperand;

typedef struct {
    IROpcode opcode;
    IROperand* result;
    IROperand* arg1;
    IROperand* arg2;
    char* label;
} IRInstruction;

typedef struct {
    char* name;
    DynamicArray params; 
    DynamicArray instructions; 
    int temp_counter;
    int label_counter;
} IRFunction;

typedef struct {
    DynamicArray functions;
} IRProgram;

IROperand* ir_operand_temp(int temp_id);
IROperand* ir_operand_var(const char* var_name);
IROperand* ir_operand_const(int64_t value);
IROperand* ir_operand_label(const char* label_name);

IRInstruction* ir_instruction_nop(void);
IRInstruction* ir_instruction_label(const char* label);
IRInstruction* ir_instruction_move(IROperand* result, IROperand* source);
IRInstruction* ir_instruction_binary(IROpcode opcode, IROperand* result, IROperand* arg1, IROperand* arg2);
IRInstruction* ir_instruction_unary(IROpcode opcode, IROperand* result, IROperand* arg);
IRInstruction* ir_instruction_jump(const char* label);
IRInstruction* ir_instruction_jump_if(IROperand* condition, const char* label);
IRInstruction* ir_instruction_jump_if_false(IROperand* condition, const char* label);
IRInstruction* ir_instruction_call(IROperand* result, const char* func_name);
IRInstruction* ir_instruction_return(IROperand* value);
IRInstruction* ir_instruction_param(IROperand* param);
IRInstruction* ir_instruction_print_op(IROperand* value);

IRFunction* ir_function_create(const char* name);
IRProgram* ir_program_create(void);

IRProgram* ir_generate(Program* ast_program);
IRFunction* ir_generate_function(Function* func);
void ir_generate_statement(IRFunction* ir_func, Stmt* stmt);
IROperand* ir_generate_expression(IRFunction* ir_func, Expr* expr);

void ir_function_add_instruction(IRFunction* func, IRInstruction* instr);
void ir_function_add_param(IRFunction* func, IROperand* param);
void ir_program_add_function(IRProgram* program, IRFunction* func);

void ir_operand_destroy(IROperand* operand);
void ir_instruction_destroy(IRInstruction* instr);
void ir_function_destroy(IRFunction* func);
void ir_program_destroy(IRProgram* program);

void ir_operand_print(const IROperand* operand);
void ir_instruction_print(const IRInstruction* instr);
void ir_function_print(const IRFunction* func);
void ir_program_print(const IRProgram* program);

const char* ir_opcode_to_string(IROpcode opcode);
int ir_function_new_temp(IRFunction* func);
char* ir_function_new_label(IRFunction* func);

#endif 