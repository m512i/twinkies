#include "common.h"
#include "backend/ir.h"
#include "frontend/ast.h"
#include "backend/codegen_core.h"
#include "backend/codegen_strategy.h"
#include "backend/codegen_instruction_handlers.h"
#include "backend/codegen_ffi.h"
#include "backend/codegen_c_writer.h"
#include "flags.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

extern bool debug_enabled;

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
const char *codegen_core_get_c_type_string(DataType type);
bool codegen_core_is_array_variable(CodeGenerator *generator, const char *var_name);
int codegen_core_get_array_size(CodeGenerator *generator, const char *var_name);
void codegen_core_write_function_header(CodeGenerator *generator, IRFunction *func);
void codegen_core_write_function_footer(CodeGenerator *generator);
void codegen_core_write_main_function(CodeGenerator *generator);
void codegen_core_error(CodeGenerator *generator, const char *message);
void codegen_core_write_operand(CodeGenerator *generator, IROperand *operand);

CodeGenerator *codegen_create(IRProgram *ir_program, Program *program, FILE *output_file, Error *error)
{
    return codegen_core_create(ir_program, program, output_file, error);
}

void codegen_destroy(CodeGenerator *generator)
{
    codegen_core_destroy(generator);
}

bool codegen_generate(CodeGenerator *generator)
{
    return codegen_core_generate(generator);
}

void codegen_generate_program(CodeGenerator *generator)
{
    codegen_core_generate_program(generator);
}

void codegen_generate_function(CodeGenerator *generator, IRFunction *func)
{
    codegen_core_generate_function(generator, func);
}

void codegen_generate_instruction(CodeGenerator *generator, IRInstruction *instr)
{
    codegen_instruction_handlers_generate_instruction(generator, instr);
}

void codegen_write_indent(CodeGenerator *generator)
{
    codegen_c_writer_write_indent(generator);
}

void codegen_write_line(CodeGenerator *generator, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    codegen_c_writer_write_line(generator, format, args);
    va_end(args);
}

void codegen_write_runtime_functions(CodeGenerator *generator)
{
    codegen_c_writer_write_runtime_functions(generator);
}

void codegen_write_header(CodeGenerator *generator)
{
    codegen_c_writer_write_header(generator);
}

void codegen_write_ffi_declarations(CodeGenerator *generator, Program *program)
{
    codegen_ffi_write_declarations(generator, program);
}

void codegen_write_ffi_loading(CodeGenerator *generator, Program *program)
{
    codegen_ffi_write_loading(generator, program);
}

const char *get_c_type_string(DataType type)
{
    return codegen_c_writer_get_c_type_string(type);
}

bool codegen_is_array_variable(CodeGenerator *generator, const char *var_name)
{
    return codegen_core_is_array_variable(generator, var_name);
}

int codegen_get_array_size(CodeGenerator *generator, const char *var_name)
{
    return codegen_core_get_array_size(generator, var_name);
}

void codegen_write_function_header(CodeGenerator *generator, IRFunction *func)
{
    codegen_c_writer_write_function_header(generator, func);
}

void codegen_write_function_footer(CodeGenerator *generator)
{
    codegen_c_writer_write_function_footer(generator);
}

void codegen_write_main_function(CodeGenerator *generator)
{
    codegen_c_writer_write_main_function(generator);
}

void codegen_error(CodeGenerator *generator, const char *message)
{
    codegen_core_error(generator, message);
}

void codegen_write_operand(CodeGenerator *generator, IROperand *operand)
{
    codegen_c_writer_write_operand(generator, operand);
}