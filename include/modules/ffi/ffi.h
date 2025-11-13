#ifndef FFI_H
#define FFI_H

#include "common/common.h"
#include "frontend/ast/ast.h"
#include "analysis/semantic/semantic.h"
#include "backend/codegen/codegen.h"
#include "backend/ir/ir.h"
#include "modules/ffi/fficonfig.h"

typedef enum
{
    FFI_CDECL,
    FFI_STDCALL,
    FFI_FASTCALL,
    FFI_THISCALL
} FFICallingConvention;

typedef enum
{
    FFI_LIBRARY_DLL,
    FFI_LIBRARY_SO,
    FFI_LIBRARY_DYLIB,
    FFI_LIBRARY_STATIC
} FFILibraryType;

typedef struct
{
    char *name;
    char *library_path;
    FFILibraryType library_type;
    char *calling_convention;
    DynamicArray functions;
    bool loaded;
    void *handle; // platform specific lib handle
} FFILibrary;

typedef struct
{
    DynamicArray libraries;
    DynamicArray functions;
    HashTable *library_map;
    HashTable *function_map;
    bool verbose;
} FFIManager;

FFIManager *ffi_manager_create(void);
void ffi_manager_destroy(FFIManager *manager);

FFILibrary *ffi_library_create(const char *name, const char *path, FFILibraryType type);
void ffi_library_destroy(FFILibrary *library);
bool ffi_library_load(FFIManager *manager, FFILibrary *library);
bool ffi_library_unload(FFIManager *manager, FFILibrary *library);
FFILibrary *ffi_manager_get_library(FFIManager *manager, const char *name);

// FFIFunction functions are now defined in fficonfig.h
FFIFunction *ffi_manager_get_function(FFIManager *manager, const char *name);

const char *ffi_get_c_type(DataType twink_type);
const char *ffi_get_ffi_type(DataType twink_type);
bool ffi_is_ffi_compatible_type(DataType type);

void ffi_generate_library_loading(CodeGenerator *generator, FFILibrary *library);
// ffi_generate_function_call removed - dynamic FFI system generates calls at code generation time
void ffi_write_platform_headers(CodeGenerator *generator);

void ffi_error_library_not_found(const char *library_name, int line, int column);
void ffi_error_function_not_found(const char *function_name, const char *library_name, int line, int column);
void ffi_error_calling_convention_mismatch(const char *function_name, int line, int column);
void ffi_error_type_mismatch(const char *function_name, DataType expected, DataType actual, int line, int column);

#ifdef _WIN32
    #define FFI_PLATFORM_WINDOWS
    void *ffi_windows_load_library(const char *path);
    void *ffi_windows_get_proc_address(void *handle, const char *name);
    void ffi_windows_free_library(void *handle);
#elif defined(__linux__)
    #define FFI_PLATFORM_LINUX
    void *ffi_linux_load_library(const char *path);
    void *ffi_linux_get_proc_address(void *handle, const char *name);
    void ffi_linux_free_library(void *handle);
#elif defined(__APPLE__)
    #define FFI_PLATFORM_MACOS
    void *ffi_macos_load_library(const char *path);
    void *ffi_macos_get_proc_address(void *handle, const char *name);
    void ffi_macos_free_library(void *handle);
#endif

const char *ffi_calling_convention_to_string(FFICallingConvention conv);
const char *ffi_library_type_to_string(FFILibraryType type);
char *ffi_resolve_library_path(const char *name, FFILibraryType type);

#endif
