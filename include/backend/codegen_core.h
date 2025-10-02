#ifndef CODEGEN_CORE_H
#define CODEGEN_CORE_H

#include "common.h"
#include "ir.h"
#include "frontend/ast.h"
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
};

CodeGenerator *codegen_core_create(IRProgram *ir_program, Program *program, FILE *output_file, Error *error);
void codegen_core_destroy(CodeGenerator *generator);
bool codegen_core_generate(CodeGenerator *generator);
void codegen_core_generate_program(CodeGenerator *generator);
void codegen_core_generate_function(CodeGenerator *generator, IRFunction *func);
void codegen_core_write_indent(CodeGenerator *generator);
void codegen_core_write_line(CodeGenerator *generator, const char *format, ...);
void codegen_core_error(CodeGenerator *generator, const char *message);
const char *codegen_core_get_c_type_string(DataType type);
bool codegen_core_is_array_variable(CodeGenerator *generator, const char *var_name);
int codegen_core_get_array_size(CodeGenerator *generator, const char *var_name);

#endif