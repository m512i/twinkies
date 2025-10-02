#ifndef FFI_CONFIG_H
#define FFI_CONFIG_H

#include <stddef.h>
#include "common.h"

typedef struct Parameter Parameter;

typedef struct {
    char *name;           
    char *library;        
    char *calling_convention; 
    int return_type;      
    void *params;         
    int line;            
    int column;          
} FFIFunction;

FFIFunction* ffi_function_create(const char *name, const char *library, const char *calling_convention, int return_type);
void ffi_function_destroy(FFIFunction *func);
void ffi_function_add_param(FFIFunction *func, Parameter *param);

const char* ffi_twink_to_c_type(int twink_type);
const char* ffi_get_calling_convention_prefix(const char *calling_convention);

#endif 
