#include "backend/codegen/codegenFfi.h"
#include "modules/ffi/ffiConfig.h"
#include <stdio.h>
#include <string.h>

void codegen_ffi_write_declarations(CodeGenerator *generator, Program *program)
{
    if (!program || program->ffi_functions.size == 0)
        return;
    
    fprintf(generator->output_file, "// FFI Function Pointers\n");
    
    for (size_t i = 0; i < program->ffi_functions.size; i++)
    {
        FFIFunction *ffi_func = (FFIFunction *)array_get(&program->ffi_functions, i);
        
        const char *return_type = ffi_twink_to_c_type(ffi_func->return_type);
        
        fprintf(generator->output_file, "typedef %s (*%s_func_t)(", return_type, ffi_func->name);
        
        DynamicArray *params = (DynamicArray*)ffi_func->params;
        if (params->size == 0)
        {
            fprintf(generator->output_file, "void");
        }
        else
        {
            for (size_t j = 0; j < params->size; j++)
            {
                if (j > 0)
                    fprintf(generator->output_file, ", ");
                Parameter *param = (Parameter *)array_get(params, j);
                const char *param_type = ffi_twink_to_c_type(param->type);
                fprintf(generator->output_file, "%s", param_type);
            }
        }
        
        fprintf(generator->output_file, ");\n");
        
        fprintf(generator->output_file, "%s_func_t ffi_%s;\n", ffi_func->name, ffi_func->name);
    }
    
    fprintf(generator->output_file, "\n");
}

void codegen_ffi_write_loading(CodeGenerator *generator, Program *program)
{
    if (!program || program->ffi_functions.size == 0)
        return;
    
    fprintf(generator->output_file, "// FFI Dynamic Loading\n");
    
    fprintf(generator->output_file, "void load_ffi_functions() {\n");
    generator->indent_level++;
    
    HashTable *loaded_libs = hashtable_create(8);
    
    for (size_t i = 0; i < program->ffi_functions.size; i++)
    {
        FFIFunction *ffi_func = (FFIFunction *)array_get(&program->ffi_functions, i);
        
        char *lib_var_name = safe_malloc(strlen(ffi_func->library) + 16);
        char *lib_name_clean = safe_malloc(strlen(ffi_func->library) + 1);
        strcpy(lib_name_clean, ffi_func->library);
        for (char *p = lib_name_clean; *p; p++) {
            if (*p == '.') *p = '_';
        }
        snprintf(lib_var_name, strlen(ffi_func->library) + 16, "%s_handle", lib_name_clean);
        safe_free(lib_name_clean);
        
        if (!hashtable_get(loaded_libs, ffi_func->library))
        {
            codegen_core_write_indent(generator);
            fprintf(generator->output_file, "void* %s = LoadLibraryA(\"%s\");\n", lib_var_name, ffi_func->library);
            
            codegen_core_write_indent(generator);
            fprintf(generator->output_file, "if (!%s) {\n", lib_var_name);
            generator->indent_level++;
            codegen_core_write_indent(generator);
            fprintf(generator->output_file, "fprintf(stderr, \"Failed to load library: %s\\n\");\n", ffi_func->library);
            codegen_core_write_indent(generator);
            fprintf(generator->output_file, "exit(1);\n");
            generator->indent_level--;
            codegen_core_write_indent(generator);
            fprintf(generator->output_file, "}\n");
            
            hashtable_put(loaded_libs, ffi_func->library, (void*)1);
        }
        
        codegen_core_write_indent(generator);
        fprintf(generator->output_file, "void* %s_ptr = GetProcAddress(%s, \"%s\");\n", 
                ffi_func->name, lib_var_name, ffi_func->name);
        
        codegen_core_write_indent(generator);
        fprintf(generator->output_file, "if (!%s_ptr) {\n", ffi_func->name);
        generator->indent_level++;
        codegen_core_write_indent(generator);
        fprintf(generator->output_file, "fprintf(stderr, \"Failed to resolve function: %s\\n\");\n", ffi_func->name);
        codegen_core_write_indent(generator);
        fprintf(generator->output_file, "exit(1);\n");
        generator->indent_level--;
        codegen_core_write_indent(generator);
        fprintf(generator->output_file, "}\n");
        
        codegen_core_write_indent(generator);
        fprintf(generator->output_file, "ffi_%s = (%s_func_t)%s_ptr;\n", 
                ffi_func->name, ffi_func->name, ffi_func->name);
        
        safe_free(lib_var_name);
    }
    
    hashtable_destroy(loaded_libs);
    
    generator->indent_level--;
    fprintf(generator->output_file, "}\n\n");
}

bool codegen_is_ffi_function(CodeGenerator *generator, const char *func_name)
{
    if (!generator->program) return false;
    
    for (size_t i = 0; i < generator->program->ffi_functions.size; i++) {
        FFIFunction *ffi_func = (FFIFunction *)array_get(&generator->program->ffi_functions, i);
        if (strcmp(ffi_func->name, func_name) == 0) {
            return true;
        }
    }
    return false;
}

const char *codegen_get_ffi_function_prefix(const char *func_name)
{
    if (strcmp(func_name, "concat") == 0 ||
        strcmp(func_name, "substr") == 0 ||
        strcmp(func_name, "strlen") == 0 ||
        strcmp(func_name, "strcmp") == 0 ||
        strcmp(func_name, "char_at") == 0)
    {
        return "__tl_";
    }
    return "";
}
