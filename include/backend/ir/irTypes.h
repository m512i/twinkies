#ifndef IR_TYPES_H
#define IR_TYPES_H

#include "common/common.h"
#include "frontend/ast/ast.h"
#include "analysis/semantic/semantic.h"

typedef enum {
    IR_OP_TEMP,
    IR_OP_VAR,
    IR_OP_CONST,
    IR_OP_STRING_CONST,
    IR_OP_LABEL,
    IR_OP_NULL
} IROperandType;

typedef struct IROperand {
    IROperandType type;
    DataType data_type;
    int array_size;
    bool is_float_const;
    union {
        int temp_id;
        char *var_name;
        int64_t const_value;
        double float_const_value;
        char *string_const_value;
        char *label_name;
    } data;
} IROperand;

typedef enum {
    IR_NOP,
    IR_LABEL,
    IR_MOVE,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_MOD,
    IR_EQ,
    IR_NE,
    IR_LT,
    IR_LE,
    IR_GT,
    IR_GE,
    IR_AND,
    IR_OR,
    IR_NOT,
    IR_NEG,
    IR_JUMP,
    IR_JUMP_IF,
    IR_JUMP_IF_FALSE,
    IR_CALL,
    IR_RETURN,
    IR_PARAM,
    IR_PRINT,
    IR_PRINT_MULTIPLE,
    IR_ARRAY_LOAD,
    IR_ARRAY_STORE,
    IR_BOUNDS_CHECK,
    IR_ARRAY_DECL,
    IR_ARRAY_INIT,
    IR_VAR_DECL,
    IR_INLINE_ASM
} IROpcode;

typedef struct IRInstruction {
    IROpcode opcode;
    IROperand *result;
    IROperand *arg1;
    IROperand *arg2;
    char *label;
    char *func_name;
    char *array_name;
    int array_size;
    DataType element_type;
    DynamicArray *args;
    char *asm_code;
    DynamicArray *asm_outputs;  
    DynamicArray *asm_inputs;   
    DynamicArray *asm_clobbers; 
    bool asm_volatile;
} IRInstruction;

typedef struct LoopContext {
    char *start_label;
    char *end_label;
    struct LoopContext *parent;
} LoopContext;

typedef struct IRFunction {
    char *name;
    DataType return_type;
    DynamicArray params;
    DynamicArray instructions;
    DynamicArray loop_stack;
    int temp_counter;
    int label_counter;
    char *oob_error_label;
} IRFunction;

typedef struct IRProgram {
    DynamicArray functions;
} IRProgram;

#endif
