#include "modules/ffi/ffi.h"
#include "backend/codegen.h"
#include "backend/ir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef FFI_PLATFORM_WINDOWS
#include <windows.h>
#elif defined(FFI_PLATFORM_LINUX) || defined(FFI_PLATFORM_MACOS)
#include <dlfcn.h>
#endif

extern bool debug_enabled;

FFIManager *ffi_manager_create(void)
{
    FFIManager *manager = safe_malloc(sizeof(FFIManager));
    array_init(&manager->libraries, 8);
    array_init(&manager->functions, 16);
    manager->library_map = hashtable_create(16);
    manager->function_map = hashtable_create(32);
    manager->verbose = false;
    return manager;
}

void ffi_manager_destroy(FFIManager *manager)
{
    if (!manager)
        return;

    for (size_t i = 0; i < manager->libraries.size; i++)
    {
        FFILibrary *lib = (FFILibrary *)array_get(&manager->libraries, i);
        ffi_library_destroy(lib);
    }
    array_free(&manager->libraries);

    for (size_t i = 0; i < manager->functions.size; i++)
    {
        FFIFunction *func = (FFIFunction *)array_get(&manager->functions, i);
        ffi_function_destroy(func);
    }
    array_free(&manager->functions);

    hashtable_destroy(manager->library_map);
    hashtable_destroy(manager->function_map);
    safe_free(manager);
}

FFILibrary *ffi_library_create(const char *name, const char *path, FFILibraryType type)
{
    FFILibrary *library = safe_malloc(sizeof(FFILibrary));
    library->name = string_copy(name);
    library->library_path = string_copy(path);
    library->library_type = type;
    library->loaded = false;
    library->handle = NULL;
    array_init(&library->functions, 8);
    return library;
}

void ffi_library_destroy(FFILibrary *library)
{
    if (!library)
        return;

    safe_free(library->name);
    safe_free(library->library_path);
    
    for (size_t i = 0; i < library->functions.size; i++)
    {
        FFIFunction *func = (FFIFunction *)array_get(&library->functions, i);
        ffi_function_destroy(func);
    }
    array_free(&library->functions);
    
    safe_free(library);
}

bool ffi_library_load(FFIManager *manager, FFILibrary *library)
{
    if (library->loaded)
        return true;

#ifdef FFI_PLATFORM_WINDOWS
    library->handle = ffi_windows_load_library(library->library_path);
#elif defined(FFI_PLATFORM_LINUX)
    library->handle = ffi_linux_load_library(library->library_path);
#elif defined(FFI_PLATFORM_MACOS)
    library->handle = ffi_macos_load_library(library->library_path);
#else
    if (manager->verbose)
    {
        printf("[FFI] Platform not supported for dynamic library loading\n");
    }
    return false;
#endif

    if (!library->handle)
    {
        if (manager->verbose)
        {
            printf("[FFI] Failed to load library: %s\n", library->library_path);
        }
        return false;
    }

    library->loaded = true;
    if (manager->verbose)
    {
        printf("[FFI] Successfully loaded library: %s\n", library->name);
    }
    return true;
}

bool ffi_library_unload(FFIManager *manager, FFILibrary *library)
{
    (void)manager; 
    if (!library->loaded)
        return true;

#ifdef FFI_PLATFORM_WINDOWS
    ffi_windows_free_library(library->handle);
#elif defined(FFI_PLATFORM_LINUX)
    ffi_linux_free_library(library->handle);
#elif defined(FFI_PLATFORM_MACOS)
    ffi_macos_free_library(library->handle);
#endif

    library->loaded = false;
    library->handle = NULL;
    return true;
}

FFILibrary *ffi_manager_get_library(FFIManager *manager, const char *name)
{
    return (FFILibrary *)hashtable_get(manager->library_map, name);
}

// FFIFunction functions are now implemented in fficonfig.c

// ffi_function_resolve removed - dynamic FFI system handles resolution at code generation time

FFIFunction *ffi_manager_get_function(FFIManager *manager, const char *name)
{
    return (FFIFunction *)hashtable_get(manager->function_map, name);
}

const char *ffi_get_c_type(DataType twink_type)
{
    switch (twink_type)
    {
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

const char *ffi_get_ffi_type(DataType twink_type)
{
    switch (twink_type)
    {
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

bool ffi_is_ffi_compatible_type(DataType type)
{
    return (type == TYPE_INT || type == TYPE_FLOAT || type == TYPE_DOUBLE || 
            type == TYPE_BOOL || type == TYPE_STRING || type == TYPE_NULL);
}

void ffi_generate_library_loading(CodeGenerator *generator, FFILibrary *library)
{
    if (!generator || !library)
        return;

    char *var_name = safe_malloc(strlen(library->name) + 16);
    snprintf(var_name, strlen(library->name) + 16, "%s_handle", library->name);

    codegen_write_indent(generator);
    fprintf(generator->output_file, "void* %s = ", var_name);

#ifdef FFI_PLATFORM_WINDOWS
    fprintf(generator->output_file, "LoadLibraryA(\"%s\");\n", library->library_path);
#elif defined(FFI_PLATFORM_LINUX) || defined(FFI_PLATFORM_MACOS)
    fprintf(generator->output_file, "dlopen(\"%s\", RTLD_LAZY);\n", library->library_path);
#endif

    codegen_write_indent(generator);
    fprintf(generator->output_file, "if (!%s) {\n", var_name);
    generator->indent_level++;
    codegen_write_indent(generator);
    fprintf(generator->output_file, "fprintf(stderr, \"Failed to load library: %s\\n\");\n", library->name);
    codegen_write_indent(generator);
    fprintf(generator->output_file, "return 1;\n");
    generator->indent_level--;
    codegen_write_indent(generator);
    fprintf(generator->output_file, "}\n");

    safe_free(var_name);
}

// ffi_generate_function_call removed - dynamic FFI system generates calls at code generation time

void ffi_write_platform_headers(CodeGenerator *generator)
{
    if (!generator)
        return;

#ifdef FFI_PLATFORM_WINDOWS
    fprintf(generator->output_file, "#include <windows.h>\n");
#elif defined(FFI_PLATFORM_LINUX) || defined(FFI_PLATFORM_MACOS)
    fprintf(generator->output_file, "#include <dlfcn.h>\n");
#endif
}

void ffi_error_library_not_found(const char *library_name, int line, int column)
{
    printf("FFI Error: Library '%s' not found (line %d, column %d)\n", 
           library_name, line, column);
}

void ffi_error_function_not_found(const char *function_name, const char *library_name, int line, int column)
{
    printf("FFI Error: Function '%s' not found in library '%s' (line %d, column %d)\n", 
           function_name, library_name, line, column);
}

void ffi_error_calling_convention_mismatch(const char *function_name, int line, int column)
{
    printf("FFI Error: Calling convention mismatch for function '%s' (line %d, column %d)\n", 
           function_name, line, column);
}

void ffi_error_type_mismatch(const char *function_name, DataType expected, DataType actual, int line, int column)
{
    printf("FFI Error: Type mismatch for function '%s' - expected %s, got %s (line %d, column %d)\n", 
           function_name, data_type_to_string(expected), data_type_to_string(actual), line, column);
}

#ifdef FFI_PLATFORM_WINDOWS
void *ffi_windows_load_library(const char *path)
{
    return LoadLibraryA(path);
}

void *ffi_windows_get_proc_address(void *handle, const char *name)
{
    return GetProcAddress((HMODULE)handle, name);
}

void ffi_windows_free_library(void *handle)
{
    FreeLibrary((HMODULE)handle);
}
#elif defined(FFI_PLATFORM_LINUX) || defined(FFI_PLATFORM_MACOS)
void *ffi_linux_load_library(const char *path)
{
    return dlopen(path, RTLD_LAZY);
}

void *ffi_linux_get_proc_address(void *handle, const char *name)
{
    return dlsym(handle, name);
}

void ffi_linux_free_library(void *handle)
{
    dlclose(handle);
}

void *ffi_macos_load_library(const char *path)
{
    return dlopen(path, RTLD_LAZY);
}

void *ffi_macos_get_proc_address(void *handle, const char *name)
{
    return dlsym(handle, name);
}

void ffi_macos_free_library(void *handle)
{
    dlclose(handle);
}
#endif

const char *ffi_calling_convention_to_string(FFICallingConvention conv)
{
    switch (conv)
    {
    case FFI_CDECL:
        return "cdecl";
    case FFI_STDCALL:
        return "stdcall";
    case FFI_FASTCALL:
        return "fastcall";
    case FFI_THISCALL:
        return "thiscall";
    default:
        return "unknown";
    }
}

const char *ffi_library_type_to_string(FFILibraryType type)
{
    switch (type)
    {
    case FFI_LIBRARY_DLL:
        return "dll";
    case FFI_LIBRARY_SO:
        return "so";
    case FFI_LIBRARY_DYLIB:
        return "dylib";
    case FFI_LIBRARY_STATIC:
        return "static";
    default:
        return "unknown";
    }
}

char *ffi_resolve_library_path(const char *name, FFILibraryType type)
{
    char *path = safe_malloc(strlen(name) + 16);
    
    switch (type)
    {
    case FFI_LIBRARY_DLL:
        snprintf(path, strlen(name) + 16, "%s.dll", name);
        break;
    case FFI_LIBRARY_SO:
        snprintf(path, strlen(name) + 16, "lib%s.so", name);
        break;
    case FFI_LIBRARY_DYLIB:
        snprintf(path, strlen(name) + 16, "lib%s.dylib", name);
        break;
    case FFI_LIBRARY_STATIC:
        snprintf(path, strlen(name) + 16, "lib%s.a", name);
        break;
    default:
        strcpy(path, name);
        break;
    }
    
    return path;
}
