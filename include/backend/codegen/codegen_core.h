#ifndef CODEGEN_CORE_H
#define CODEGEN_CORE_H

#include "common/common.h"
#include "backend/ir/ir.h"
#include "frontend/ast/ast.h"
#include <stdio.h>

#define MAX_PARAMS 16

typedef struct CodeGenerator CodeGenerator;
typedef struct CodeGenStrategy CodeGenStrategy;

struct CodeGenerator {
    IRProgram *ir_program;
    Program *program;
    FILE *output_file;
    Error *error;
    int indent_level;
    int temp_counter;
    HashTable *temp_map;
    HashTable *var_set;
    HashTable *array_info;
    HashTable *variable_types;
    int param_count;
    IROperand *params[MAX_PARAMS];
    CodeGenStrategy *strategy;
    const char *current_function_name;
    DataType current_function_return_type;
    char epilogue_label[64];
    HashTable *declared_temps;
};

CodeGenerator *codegen_core_create(IRProgram *ir_program, Program *program, FILE *output_file, Error *error);
void codegen_core_destroy(CodeGenerator *generator);
bool codegen_core_generate(CodeGenerator *generator);
void codegen_core_generate_program(CodeGenerator *generator);
void codegen_core_generate_function(CodeGenerator *generator, IRFunction *func);
void codegen_core_generate_instruction(CodeGenerator *generator, IRInstruction *instr);
void codegen_core_write_indent(CodeGenerator *generator);
void codegen_core_write_line(CodeGenerator *generator, const char *format, ...);
void codegen_core_write_runtime_functions(CodeGenerator *generator);
void codegen_core_write_header(CodeGenerator *generator);
void codegen_core_write_ffi_declarations(CodeGenerator *generator, Program *program);
void codegen_core_write_ffi_loading(CodeGenerator *generator, Program *program);
void codegen_core_error(CodeGenerator *generator, const char *message);
const char *codegen_core_get_c_type_string(DataType type);
bool codegen_core_is_array_variable(CodeGenerator *generator, const char *var_name);
int codegen_core_get_array_size(CodeGenerator *generator, const char *var_name);
void codegen_core_write_function_header(CodeGenerator *generator, IRFunction *func);
void codegen_core_write_function_footer(CodeGenerator *generator);
void codegen_core_write_main_function(CodeGenerator *generator);
void codegen_core_write_operand(CodeGenerator *generator, IROperand *operand);

#endif