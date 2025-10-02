#ifndef CODEGEN_C_WRITER_H
#define CODEGEN_C_WRITER_H

#include "common.h"
#include "ir.h"
#include "frontend/ast.h"
#include "backend/codegen_core.h"
#include <stdio.h>

void codegen_c_writer_write_header(CodeGenerator *generator);
void codegen_c_writer_write_runtime_functions(CodeGenerator *generator);
void codegen_c_writer_write_function_header(CodeGenerator *generator, IRFunction *func);
void codegen_c_writer_write_function_footer(CodeGenerator *generator);
void codegen_c_writer_write_main_function(CodeGenerator *generator);
void codegen_c_writer_write_operand(CodeGenerator *generator, IROperand *operand);

const char *codegen_c_writer_get_c_type_string(DataType type);
const char *codegen_c_writer_get_printf_format(DataType type);
bool codegen_c_writer_is_float_type(DataType type);

void codegen_c_writer_write_indent(CodeGenerator *generator);
void codegen_c_writer_write_line(CodeGenerator *generator, const char *format, ...);

#endif