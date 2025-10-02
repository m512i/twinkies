#ifndef MODULES_H
#define MODULES_H

#include "common.h"
#include "frontend/ast.h"
#include "analysis/semantic.h"

#define MAX_INCLUDE_PATHS 32
#define MAX_MODULE_DEPENDENCIES 64
#define MAX_SYMBOL_TABLE_SIZE 1024

typedef enum
{
    SYMBOL_PRIVATE,
    SYMBOL_PUBLIC
} SymbolVisibility;

typedef struct
{
    char *name;
    SymbolVisibility visibility;
    DataType type;
    int line;
    int column;
    char *module_name;
} ModuleSymbol;

typedef struct
{
    char *path;
    IncludeType type;
    int line;
    int column;
    bool resolved;
    char *resolved_path;
} IncludeDirective;

typedef struct
{
    char *name;
    char *file_path;
    char *header_path;
    char *source_path;

    DynamicArray includes;
    DynamicArray dependencies;

    DynamicArray symbols;
    DynamicArray exported_symbols;

    Program *ast;

    bool compiled;
    bool header_parsed;
    bool source_parsed;
    char *object_file;
    char *header_dependencies_file;

    time_t last_modified;
    time_t last_compiled;
} Module;

typedef struct
{
    DynamicArray modules;
    DynamicArray include_paths;
    DynamicArray system_include_paths;
    char *output_directory;
    bool verbose;
} ModuleManager;

ModuleManager *module_manager_create(void);
void module_manager_destroy(ModuleManager *manager);

Module *module_create(const char *name, const char *file_path);
void module_destroy(Module *module);
Module *module_manager_get_module(ModuleManager *manager, const char *name);
bool module_manager_add_module(ModuleManager *manager, Module *module);

void module_manager_add_include_path(ModuleManager *manager, const char *path);
void module_manager_add_system_include_path(ModuleManager *manager, const char *path);
char *module_manager_resolve_include(ModuleManager *manager, const char *include_path, IncludeType type);

bool module_manager_build_dependencies(ModuleManager *manager, Module *module);
bool module_needs_recompilation(Module *module);
void module_update_timestamps(Module *module);

bool module_add_symbol(Module *module, const char *name, SymbolVisibility visibility, DataType type, int line, int column);
ModuleSymbol *module_find_symbol(Module *module, const char *name);
bool module_export_symbol(Module *module, const char *name);

bool module_parse_header(ModuleManager *manager, Module *module);
bool module_compile_header(ModuleManager *manager, Module *module);
bool module_compile_source(ModuleManager *manager, Module *module);
bool module_manager_compile_all(ModuleManager *manager);
bool module_manager_link(ModuleManager *manager, const char *output_file);

char *module_get_object_file_path(ModuleManager *manager, Module *module);
char *module_get_dependencies_file_path(ModuleManager *manager, Module *module);
void module_print_dependencies(Module *module);

char *get_module_name_from_path(const char *file_path);
void semantic_add_global_symbol(SemanticAnalyzer *analyzer, const char *name, SymbolType type, DataType data_type);
void semantic_add_global_function_with_params(SemanticAnalyzer *analyzer, Function *func);

#endif