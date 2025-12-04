#ifndef CODEGEN_FFI_H
#define CODEGEN_FFI_H

#include "common/common.h"
#include "frontend/ast/ast.h"
#include "backend/codegen/codegenCore.h"
#include <stdio.h>

void codegen_ffi_write_declarations(CodeGenerator *generator, Program *program);
void codegen_ffi_write_loading(CodeGenerator *generator, Program *program);

bool codegen_is_ffi_function(CodeGenerator *generator, const char *func_name);
const char *codegen_get_ffi_function_prefix(const char *func_name);

const char *ffi_twink_to_c_type(int twink_type);
const char *ffi_get_calling_convention_prefix(const char *calling_convention);

#endif