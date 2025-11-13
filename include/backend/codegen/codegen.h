#ifndef CODEGEN_H
#define CODEGEN_H

#include "common/common.h"
#include "backend/ir/ir.h"
#include "frontend/ast/ast.h"

#define MAX_PARAMS 16

typedef struct
{
    char *name;
    int size;
} ArrayInfo;

typedef struct
{
    char *name;
    DataType type;
} VariableInfo;


CodeGenerator *codegen_create(IRProgram *ir_program, Program *program, FILE *output_file, Error *error);
void codegen_destroy(CodeGenerator *generator);
bool codegen_generate(CodeGenerator *generator);
void codegen_generate_program(CodeGenerator *generator);
void codegen_generate_function(CodeGenerator *generator, IRFunction *func);
void codegen_generate_instruction(CodeGenerator *generator, IRInstruction *instr);
void codegen_write_indent(CodeGenerator *generator);
void codegen_write_line(CodeGenerator *generator, const char *format, ...);
void codegen_write_operand(CodeGenerator *generator, IROperand *operand);
char *codegen_get_temp_name(CodeGenerator *generator, IROperand *operand);
void codegen_write_runtime_functions(CodeGenerator *generator);
void codegen_write_header(CodeGenerator *generator);
void codegen_write_ffi_declarations(CodeGenerator *generator, Program *program);
void codegen_write_ffi_loading(CodeGenerator *generator, Program *program);
void codegen_write_function_header(CodeGenerator *generator, IRFunction *func);
void codegen_write_function_footer(CodeGenerator *generator);
void codegen_write_main_function(CodeGenerator *generator);

CodeGenerator *codegenasm_create(IRProgram *ir_program, FILE *output_file, Error *error);
void codegenasm_destroy(CodeGenerator *generator);
bool codegenasm_generate(CodeGenerator *generator);

void codegenasm_generate_program(CodeGenerator *generator);
void codegenasm_generate_function(CodeGenerator *generator, IRFunction *func);
void codegenasm_generate_instruction(CodeGenerator *generator, IRInstruction *instr);

void codegenasm_move(CodeGenerator *generator, IROperand *dest, IROperand *src);
void codegenasm_binary_op(CodeGenerator *generator, const char *op, IROperand *result, IROperand *arg1, IROperand *arg2);
void codegenasm_unary_op(CodeGenerator *generator, const char *op, IROperand *result, IROperand *arg);
void codegenasm_mul(CodeGenerator *generator, IROperand *result, IROperand *arg1, IROperand *arg2);
void codegenasm_div(CodeGenerator *generator, IROperand *result, IROperand *arg1, IROperand *arg2);
void codegenasm_mod(CodeGenerator *generator, IROperand *result, IROperand *arg1, IROperand *arg2);
void codegenasm_not(CodeGenerator *generator, IROperand *result, IROperand *arg);
void codegenasm_compare(CodeGenerator *generator, const char *set_op, IROperand *result, IROperand *arg1, IROperand *arg2);
void codegenasm_call(CodeGenerator *generator, IROperand *result, const char *func_name);
void codegenasm_print(CodeGenerator *generator, IROperand *value);
char *codegenasm_get_operand_name(CodeGenerator *generator, IROperand *operand);
char *codegenasm_get_temp_name(CodeGenerator *generator, IROperand *operand);
char *codegenasm_get_const_name(CodeGenerator *generator, IROperand *operand);

void codegenasm_write_header(CodeGenerator *generator);
void codegenasm_write_data_section(CodeGenerator *generator);
void codegenasm_write_function_header(CodeGenerator *generator, IRFunction *func);
void codegenasm_write_function_footer(CodeGenerator *generator);
void codegenasm_write_main_function(CodeGenerator *generator);

void codegen_error(CodeGenerator *generator, const char *message);
void codegenasm_error(CodeGenerator *generator, const char *message);

void codegenasm_array_load(CodeGenerator *generator, IROperand *result, IROperand *array, IROperand *index);
void codegenasm_array_store(CodeGenerator *generator, IROperand *array, IROperand *index, IROperand *value);
void codegenasm_bounds_check(CodeGenerator *generator, IROperand *index, IROperand *size, const char *error_label);

#endif