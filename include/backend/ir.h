#ifndef IR_H
#define IR_H

#include "common.h"
#include "frontend/ast.h"
#include "analysis/semantic.h"

typedef enum
{
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
    IR_PRINT,
    IR_ARRAY_LOAD,
    IR_ARRAY_STORE,
    IR_BOUNDS_CHECK,
    IR_ARRAY_DECL,
    IR_ARRAY_INIT,
    IR_VAR_DECL
} IROpcode;

typedef enum
{
    IR_OP_TEMP,
    IR_OP_VAR,
    IR_OP_CONST,
    IR_OP_STRING_CONST,
    IR_OP_LABEL,
    IR_OP_NULL
} IROperandType;

typedef struct
{
    IROperandType type;
    int array_size;
    bool is_float_const;
    DataType data_type;
    union
    {
        int temp_id;
        char *var_name;
        int64_t const_value;
        double float_const_value;
        char *string_const_value;
        char *label_name;
    } data;
} IROperand;

typedef struct
{
    IROpcode opcode;
    IROperand *result;
    IROperand *arg1;
    IROperand *arg2;
    DynamicArray *args; // For instructions that need multiple arguments (like PRINT)
    char *label;
} IRInstruction;

typedef struct LoopContext
{
    char *loop_start_label;
    char *loop_end_label;
    struct LoopContext *parent;
} LoopContext;

typedef struct
{
    char *name;
    DataType return_type;
    DynamicArray params;
    DynamicArray instructions;
    int temp_counter;
    int label_counter;
    LoopContext *current_loop;
} IRFunction;

typedef struct
{
    DynamicArray functions;
} IRProgram;

IRFunction *ir_function_create(const char *name, DataType return_type);
IRProgram *ir_program_create(void);

IRProgram *ir_generate(Program *ast_program, SemanticAnalyzer *analyzer);
IRProgram *ir_generate_with_modules(Program *ast_program, SemanticAnalyzer *analyzer, void *module_manager);
IRFunction *ir_generate_function(Function *func, SemanticAnalyzer *analyzer);
void ir_generate_statement(IRFunction *ir_func, Stmt *stmt, SemanticAnalyzer *analyzer);
IROperand *ir_generate_expression(IRFunction *ir_func, Expr *expr, SemanticAnalyzer *analyzer, DataType expected_type);

void ir_function_add_instruction(IRFunction *func, IRInstruction *instr);
void ir_function_add_param(IRFunction *func, IROperand *param);
void ir_program_add_function(IRProgram *program, IRFunction *func);

void ir_operand_destroy(IROperand *operand);
void ir_function_destroy(IRFunction *func);
void ir_program_destroy(IRProgram *program);

void ir_operand_print(const IROperand *operand);
void ir_function_print(const IRFunction *func);
void ir_program_print(const IRProgram *program);

const char *ir_opcode_to_string(IROpcode opcode);
int ir_function_new_temp(IRFunction *func);
char *ir_function_new_label(IRFunction *func);

LoopContext *ir_loop_context_create(char *start_label, char *end_label);
void ir_loop_context_destroy(LoopContext *context);
void ir_function_enter_loop(IRFunction *func, char *start_label, char *end_label);
void ir_function_exit_loop(IRFunction *func);
LoopContext *ir_function_get_current_loop(IRFunction *func);

#endif