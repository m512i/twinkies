#include "modules/ffi/fficonfig.h"
#include "frontend/ast/ast.h"
#include <string.h>

FFIFunction* ffi_function_create(const char *name, const char *library, const char *calling_convention, int return_type) {
    FFIFunction *func = safe_malloc(sizeof(FFIFunction));
    func->name = string_copy(name);
    func->library = string_copy(library);
    func->calling_convention = string_copy(calling_convention);
    func->return_type = return_type;
    func->params = safe_malloc(sizeof(DynamicArray));
    array_init((DynamicArray*)func->params, 4);
    func->line = 0;
    func->column = 0;
    return func;
}

void ffi_function_destroy(FFIFunction *func) {
    if (!func) return;
    
    safe_free(func->name);
    safe_free(func->library);
    safe_free(func->calling_convention);
    
    if (func->params) {
        DynamicArray *params = (DynamicArray*)func->params;
        for (size_t i = 0; i < params->size; i++) {
            Parameter *param = (Parameter *)array_get(params, i);
            parameter_destroy(param);
        }
        array_free(params);
        safe_free(func->params);
    }
    
    safe_free(func);
}

void ffi_function_add_param(FFIFunction *func, Parameter *param) {
    if (func && param && func->params) {
        array_push((DynamicArray*)func->params, param);
    }
}

const char* ffi_twink_to_c_type(int twink_type) {
    switch (twink_type) {
        case TYPE_INT:
            return "int64_t";
        case TYPE_FLOAT:
            return "float";
        case TYPE_DOUBLE:
            return "double";
        case TYPE_BOOL:
            return "bool";
        case TYPE_STRING:
            return "char*";
        case TYPE_NULL:
            return "void*";
        default:
            return "void*";
    }
}

const char* ffi_get_calling_convention_prefix(const char *calling_convention) {
    if (!calling_convention) return "";
    
    if (strcmp(calling_convention, "cdecl") == 0) {
        return "__cdecl";
    } else if (strcmp(calling_convention, "stdcall") == 0) {
        return "__stdcall";
    } else if (strcmp(calling_convention, "fastcall") == 0) {
        return "__fastcall";
    }
    
    return "";
}
