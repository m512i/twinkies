#include "modules/modules.h"
#include "common/utils.h"
#include "common/flags.h"
#include "frontend/lexer/lexer.h"
#include "frontend/parser/parser.h"
#include "frontend/ast/ast.h"
#include "analysis/semantic/semantic.h"
#include "backend/ir/ir.h"
#include "backend/codegen/codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

ModuleManager *module_manager_create(void)
{
    ModuleManager *manager = safe_malloc(sizeof(ModuleManager));
    array_init(&manager->modules, 8);
    array_init(&manager->include_paths, 4);
    array_init(&manager->system_include_paths, 4);
    manager->output_directory = NULL;
    manager->verbose = false;

    module_manager_add_include_path(manager, ".");
    module_manager_add_include_path(manager, "./include");
    module_manager_add_system_include_path(manager, "/usr/include");
    module_manager_add_system_include_path(manager, "/usr/local/include");

    return manager;
}

void module_manager_destroy(ModuleManager *manager)
{
    if (!manager)
        return;

    for (size_t i = 0; i < manager->modules.size; i++)
    {
        Module *module = (Module *)array_get(&manager->modules, i);
        module_destroy(module);
    }
    array_free(&manager->modules);

    for (size_t i = 0; i < manager->include_paths.size; i++)
    {
        char *path = (char *)array_get(&manager->include_paths, i);
        safe_free(path);
    }
    array_free(&manager->include_paths);

    for (size_t i = 0; i < manager->system_include_paths.size; i++)
    {
        char *path = (char *)array_get(&manager->system_include_paths, i);
        safe_free(path);
    }
    array_free(&manager->system_include_paths);

    safe_free(manager->output_directory);
    safe_free(manager);
}

Module *module_create(const char *name, const char *file_path)
{
    Module *module = safe_malloc(sizeof(Module));
    module->name = string_copy(name);
    module->file_path = string_copy(file_path);

    module->header_path = string_copy(file_path);

    char *source_path = string_copy(file_path);
    char *dot = strrchr(source_path, '.');
    if (dot && strcmp(dot, ".tlh") == 0)
    {
        strcpy(dot, ".tl");
        module->source_path = source_path;
    }
    else
    {
        module->source_path = string_copy(file_path);
        safe_free(source_path);
    }

    array_init(&module->includes, 4);
    array_init(&module->dependencies, 4);
    array_init(&module->symbols, 16);
    array_init(&module->exported_symbols, 8);

    module->ast = NULL;
    module->compiled = false;
    module->header_parsed = false;
    module->source_parsed = false;
    module->object_file = NULL;
    module->header_dependencies_file = NULL;
    module->last_modified = 0;
    module->last_compiled = 0;

    struct stat st;
    if (stat(file_path, &st) == 0)
    {
        module->last_modified = st.st_mtime;
    }

    return module;
}

void module_destroy(Module *module)
{
    if (!module)
        return;

    safe_free(module->name);
    safe_free(module->file_path);
    safe_free(module->header_path);
    safe_free(module->source_path);
    safe_free(module->object_file);
    safe_free(module->header_dependencies_file);

    for (size_t i = 0; i < module->includes.size; i++)
    {
        IncludeDirective *include = (IncludeDirective *)array_get(&module->includes, i);
        safe_free(include->path);
        safe_free(include->resolved_path);
        safe_free(include);
    }
    array_free(&module->includes);

    for (size_t i = 0; i < module->dependencies.size; i++)
    {
        char *dep = (char *)array_get(&module->dependencies, i);
        safe_free(dep);
    }
    array_free(&module->dependencies);

    for (size_t i = 0; i < module->symbols.size; i++)
    {
        ModuleSymbol *symbol = (ModuleSymbol *)array_get(&module->symbols, i);
        safe_free(symbol->name);
        safe_free(symbol->module_name);
        safe_free(symbol);
    }
    array_free(&module->symbols);

    for (size_t i = 0; i < module->exported_symbols.size; i++)
    {
        char *symbol_name = (char *)array_get(&module->exported_symbols, i);
        safe_free(symbol_name);
    }
    array_free(&module->exported_symbols);

    if (module->ast)
    {
        program_destroy(module->ast);
    }

    safe_free(module);
}

Module *module_manager_get_module(ModuleManager *manager, const char *name)
{
    for (size_t i = 0; i < manager->modules.size; i++)
    {
        Module *module = (Module *)array_get(&manager->modules, i);
        if (strcmp(module->name, name) == 0)
        {
            return module;
        }
    }
    return NULL;
}

bool module_manager_add_module(ModuleManager *manager, Module *module)
{
    if (module_manager_get_module(manager, module->name))
    {
        return false;
    }

    array_push(&manager->modules, module);
    return true;
}

void module_manager_add_include_path(ModuleManager *manager, const char *path)
{
    char *path_copy = string_copy(path);
    array_push(&manager->include_paths, path_copy);
}

void module_manager_add_system_include_path(ModuleManager *manager, const char *path)
{
    char *path_copy = string_copy(path);
    array_push(&manager->system_include_paths, path_copy);
}

char *module_manager_resolve_include(ModuleManager *manager, const char *include_path, IncludeType type)
{
    if (type == INCLUDE_SYSTEM)
    {
        for (size_t i = 0; i < manager->system_include_paths.size; i++)
        {
            char *system_path = (char *)array_get(&manager->system_include_paths, i);
            char *full_path = malloc(strlen(system_path) + strlen(include_path) + 2);
            sprintf(full_path, "%s/%s", system_path, include_path);

            FILE *file = fopen(full_path, "r");
            if (file)
            {
                fclose(file);
                return full_path;
            }
            free(full_path);
        }
    }
    else
    {
        for (size_t i = 0; i < manager->include_paths.size; i++)
        {
            char *local_path = (char *)array_get(&manager->include_paths, i);
            char *full_path = malloc(strlen(local_path) + strlen(include_path) + 2);
            sprintf(full_path, "%s/%s", local_path, include_path);

            FILE *file = fopen(full_path, "r");
            if (file)
            {
                fclose(file);
                return full_path;
            }
            free(full_path);
        }
    }

    return NULL;
}

bool module_manager_build_dependencies(ModuleManager *manager, Module *module)
{
    if (!module->ast)
        return false;

    for (size_t i = 0; i < module->dependencies.size; i++)
    {
        char *dep = (char *)array_get(&module->dependencies, i);
        safe_free(dep);
    }
    module->dependencies.size = 0;

    for (size_t i = 0; i < module->ast->functions.size; i++)
    {
        Function *func = (Function *)array_get(&module->ast->functions, i);
        if (func->body && func->body->type == STMT_BLOCK)
        {
            for (size_t j = 0; j < func->body->data.block.statements.size; j++)
            {
                Stmt *stmt = (Stmt *)array_get(&func->body->data.block.statements, j);
                if (stmt->type == STMT_INCLUDE)
                {
                    char *resolved_path = module_manager_resolve_include(manager, stmt->data.include.path, stmt->data.include.type);
                    if (resolved_path)
                    {
                        array_push(&module->dependencies, string_copy(resolved_path));
                        free(resolved_path);
                    }
                }
            }
        }
    }

    return true;
}

bool module_needs_recompilation(Module *module)
{
    if (!module->compiled)
        return true;

    struct stat st;
    if (stat(module->file_path, &st) == 0)
    {
        if (st.st_mtime > module->last_compiled)
        {
            return true;
        }
    }

    for (size_t i = 0; i < module->dependencies.size; i++)
    {
        char *dep_path = (char *)array_get(&module->dependencies, i);
        if (stat(dep_path, &st) == 0)
        {
            if (st.st_mtime > module->last_compiled)
            {
                return true;
            }
        }
    }

    return false;
}

void module_update_timestamps(Module *module)
{
    struct stat st;
    if (stat(module->file_path, &st) == 0)
    {
        module->last_modified = st.st_mtime;
    }
    module->last_compiled = time(NULL);
}

bool module_add_symbol(Module *module, const char *name, SymbolVisibility visibility, DataType type, int line, int column)
{
    for (size_t i = 0; i < module->symbols.size; i++)
    {
        ModuleSymbol *existing = (ModuleSymbol *)array_get(&module->symbols, i);
        if (strcmp(existing->name, name) == 0)
        {
            return false;
        }
    }

    ModuleSymbol *symbol = safe_malloc(sizeof(ModuleSymbol));
    symbol->name = string_copy(name);
    symbol->visibility = visibility;
    symbol->type = type;
    symbol->line = line;
    symbol->column = column;
    symbol->module_name = string_copy(module->name);

    array_push(&module->symbols, symbol);
    return true;
}

ModuleSymbol *module_find_symbol(Module *module, const char *name)
{
    for (size_t i = 0; i < module->symbols.size; i++)
    {
        ModuleSymbol *symbol = (ModuleSymbol *)array_get(&module->symbols, i);
        if (strcmp(symbol->name, name) == 0)
        {
            return symbol;
        }
    }
    return NULL;
}

bool module_export_symbol(Module *module, const char *name)
{
    ModuleSymbol *symbol = module_find_symbol(module, name);
    if (!symbol || symbol->visibility != SYMBOL_PUBLIC)
    {
        return false;
    }

    for (size_t i = 0; i < module->exported_symbols.size; i++)
    {
        char *exported = (char *)array_get(&module->exported_symbols, i);
        if (strcmp(exported, name) == 0)
        {
            return true;
        }
    }

    array_push(&module->exported_symbols, string_copy(name));
    return true;
}

bool module_parse_header(ModuleManager *manager, Module *module)
{
    if (module->header_parsed)
        return true;

    char *header_source = read_file(module->header_path);
    if (!header_source)
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Could not read header file: %s\n", module->header_path);
        }
        return false;
    }

    ErrorContext *error_context = error_context_create(module->header_path, header_source);
    Error error;
    error_init(&error);

    Lexer *lexer = lexer_create(header_source, &error);
    if (error.type != ERROR_NONE)
    {
        error_context_add_error(error_context, error.type, SEVERITY_ERROR,
                                error.message, error.suggestion, error.line, error.column);
        error_init(&error);
    }

    Parser *parser = NULL;
    if (lexer)
    {
        parser = parser_create(lexer, error_context);
        if (error.type != ERROR_NONE)
        {
            error_context_add_error(error_context, error.type, SEVERITY_ERROR,
                                    error.message, error.suggestion, error.line, error.column);
            error_init(&error);
        }
        else
        {
            module->ast = program_create();

            while (!parser_check(parser, TOKEN_EOF))
            {
                if (parser_match(parser, TOKEN_FUNC))
                {
                    Function *func = parse_function_declaration(parser);
                    if (func)
                    {
                        program_add_function(module->ast, func);
                        if (debug_enabled)
                        {
                            printf("[DEBUG] Added function declaration to module %s: %s\n", module->name, func->name);
                        }
                    }
                    else
                    {
                        if (debug_enabled)
                        {
                            printf("[DEBUG] Failed to parse function declaration in module %s\n", module->name);
                        }
                        break;
                    }
                }
                else
                {
                    parser_advance(parser);
                }
            }
        }
    }

    if (error_context_has_errors(error_context))
    {
        error_context_print_all(error_context);
        error_context_destroy(error_context);
        if (parser)
            parser_destroy(parser);
        if (lexer)
            lexer_destroy(lexer);
        safe_free(header_source);
        return false;
    }

    char *source_source = read_file(module->source_path);
    if (source_source)
    {
        ErrorContext *source_error_context = error_context_create(module->source_path, source_source);
        Error source_error;
        error_init(&source_error);

        Lexer *source_lexer = lexer_create(source_source, &source_error);
        if (source_error.type != ERROR_NONE)
        {
            error_context_add_error(source_error_context, source_error.type, SEVERITY_ERROR,
                                    source_error.message, source_error.suggestion, source_error.line, source_error.column);
            error_init(&source_error);
        }

        Parser *source_parser = NULL;
        if (source_lexer)
        {
            source_parser = parser_create(source_lexer, source_error_context);
            if (source_error.type != ERROR_NONE)
            {
                error_context_add_error(source_error_context, source_error.type, SEVERITY_ERROR,
                                        source_error.message, source_error.suggestion, source_error.line, source_error.column);
                error_init(&source_error);
            }
            else
            {
                while (!parser_check(source_parser, TOKEN_EOF))
                {
                    if (parser_match(source_parser, TOKEN_FUNC))
                    {
                        Function *func = parse_function(source_parser);
                        if (func)
                        {
                            for (size_t i = 0; i < module->ast->functions.size; i++)
                            {
                                Function *decl_func = (Function *)array_get(&module->ast->functions, i);
                                if (strcmp(decl_func->name, func->name) == 0)
                                {
                                    decl_func->body = func->body;
                                    func->body = NULL;
                                    if (debug_enabled)
                                    {
                                        printf("[DEBUG] Added implementation for function %s in module %s\n", func->name, module->name);
                                    }
                                    break;
                                }
                            }
                            function_destroy(func);
                        }
                        else
                        {
                            if (debug_enabled)
                            {
                                printf("[DEBUG] Failed to parse function implementation in module %s\n", module->name);
                            }
                            break;
                        }
                    }
                    else
                    {
                        parser_advance(source_parser);
                    }
                }
            }
        }

        if (error_context_has_errors(source_error_context))
        {
            error_context_print_all(source_error_context);
        }

        error_context_destroy(source_error_context);
        if (source_parser)
            parser_destroy(source_parser);
        if (source_lexer)
            lexer_destroy(source_lexer);
        safe_free(source_source);
    }
    else
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Could not read source file: %s\n", module->source_path);
        }
    }

    module_manager_build_dependencies(manager, module);

    module->header_parsed = true;
    error_context_destroy(error_context);
    if (parser)
        parser_destroy(parser);
    if (lexer)
        lexer_destroy(lexer);
    safe_free(header_source);

    return true;
}

bool module_compile_header(ModuleManager *manager, Module *module)
{
    if (module->header_parsed)
        return true;

    char *source = read_file(module->file_path);
    if (!source)
        return false;

    ErrorContext *error_context = error_context_create(module->file_path, source);
    Error error;
    error_init(&error);

    Lexer *lexer = lexer_create(source, &error);
    if (error.type != ERROR_NONE)
    {
        error_context_add_error(error_context, error.type, SEVERITY_ERROR,
                                error.message, error.suggestion, error.line, error.column);
        error_init(&error);
    }

    Parser *parser = NULL;
    if (lexer)
    {
        parser = parser_create(lexer, error_context);
        if (error.type != ERROR_NONE)
        {
            error_context_add_error(error_context, error.type, SEVERITY_ERROR,
                                    error.message, error.suggestion, error.line, error.column);
            error_init(&error);
        }
        else
        {
            module->ast = parser_parse(parser);
            if (error.type != ERROR_NONE)
            {
                error_context_add_error(error_context, error.type, SEVERITY_ERROR,
                                        error.message, error.suggestion, error.line, error.column);
                error_init(&error);
            }
        }
    }

    if (error_context_has_errors(error_context))
    {
        error_context_print_all(error_context);
        error_context_destroy(error_context);
        if (parser)
            parser_destroy(parser);
        if (lexer)
            lexer_destroy(lexer);
        safe_free(source);
        return false;
    }

    module_manager_build_dependencies(manager, module);

    module->header_parsed = true;
    error_context_destroy(error_context);
    if (parser)
        parser_destroy(parser);
    if (lexer)
        lexer_destroy(lexer);
    safe_free(source);

    return true;
}

bool module_compile_source(ModuleManager *manager, Module *module)
{
    if (!module->header_parsed)
    {
        if (!module_parse_header(manager, module))
        {
            return false;
        }
    }

    if (module->source_parsed)
        return true;

    if (module->ast)
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Module %s has %zu functions in AST\n", module->name, module->ast->functions.size);
        }

        for (size_t i = 0; i < module->ast->functions.size; i++)
        {
            Function *func = (Function *)array_get(&module->ast->functions, i);

            if (debug_enabled)
            {
                printf("[DEBUG] Processing function %s in module %s\n", func->name, module->name);
            }

            module_add_symbol(module, func->name, SYMBOL_PUBLIC, func->return_type, 0, 0);

            module_export_symbol(module, func->name);

            if (debug_enabled)
            {
                printf("[DEBUG] Exported function from module %s: %s\n", module->name, func->name);
            }
        }
    }
    else
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Module %s has no AST\n", module->name);
        }
    }

    module->source_parsed = true;
    return true;
}

bool module_manager_compile_all(ModuleManager *manager)
{
    bool success = true;

    for (size_t i = 0; i < manager->modules.size; i++)
    {
        Module *module = (Module *)array_get(&manager->modules, i);

        if (module_needs_recompilation(module))
        {
            if (manager->verbose)
            {
                printf("Compiling module: %s\n", module->name);
            }

            if (!module_compile_source(manager, module))
            {
                success = false;
                continue;
            }

            char *object_path = module_get_object_file_path(manager, module);
            if (!compile_file(module->file_path, object_path, manager->verbose, false))
            {
                success = false;
                safe_free(object_path);
                continue;
            }

            module->object_file = object_path;
            module->compiled = true;
            module_update_timestamps(module);
        }
    }

    return success;
}

bool module_manager_link(ModuleManager *manager, const char *output_file)
{
    // For now, just compile the first module as the main program
    // In a full implementation, this would link all object files together
    if (manager->modules.size == 0)
        return false;

    Module *main_module = (Module *)array_get(&manager->modules, 0);
    return compile_file(main_module->file_path, output_file, manager->verbose, false);
}

char *module_get_object_file_path(ModuleManager *manager, Module *module)
{
    (void)manager;

    char *base_name = strrchr(module->file_path, '/');
    if (!base_name)
        base_name = strrchr(module->file_path, '\\');
    if (!base_name)
        base_name = module->file_path;
    else
        base_name++;

    char *name_without_ext = string_copy(base_name);
    char *dot = strrchr(name_without_ext, '.');
    if (dot)
        *dot = '\0';

    char *object_path = malloc(strlen(name_without_ext) + 5);
    sprintf(object_path, "%s.o", name_without_ext);

    safe_free(name_without_ext);
    return object_path;
}

char *module_get_dependencies_file_path(ModuleManager *manager, Module *module)
{
    (void)manager;

    char *base_name = strrchr(module->file_path, '/');
    if (!base_name)
        base_name = strrchr(module->file_path, '\\');
    if (!base_name)
        base_name = module->file_path;
    else
        base_name++;

    char *name_without_ext = string_copy(base_name);
    char *dot = strrchr(name_without_ext, '.');
    if (dot)
        *dot = '\0';

    char *dep_path = malloc(strlen(name_without_ext) + 5);
    sprintf(dep_path, "%s.d", name_without_ext);

    safe_free(name_without_ext);
    return dep_path;
}

void module_print_dependencies(Module *module)
{
    printf("Module: %s\n", module->name);
    printf("File: %s\n", module->file_path);
    printf("Dependencies:\n");

    for (size_t i = 0; i < module->dependencies.size; i++)
    {
        char *dep = (char *)array_get(&module->dependencies, i);
        printf("  %s\n", dep);
    }

    printf("Exported symbols:\n");
    for (size_t i = 0; i < module->exported_symbols.size; i++)
    {
        char *symbol = (char *)array_get(&module->exported_symbols, i);
        printf("  %s\n", symbol);
    }
}

char *get_module_name_from_path(const char *file_path)
{
    const char *last_sep = strrchr(file_path, '/');
    if (!last_sep)
        last_sep = strrchr(file_path, '\\');

    const char *base_name = last_sep ? last_sep + 1 : file_path;

    const char *last_dot = strrchr(base_name, '.');
    size_t name_len = last_dot ? (size_t)(last_dot - base_name) : strlen(base_name);

    char *module_name = malloc(name_len + 1);
    strncpy(module_name, base_name, name_len);
    module_name[name_len] = '\0';

    return module_name;
}

static DynamicArray *get_or_create_overload_set(SemanticAnalyzer *analyzer, const char *name)
{
    DynamicArray *overloads = hashtable_get(analyzer->current_scope->symbols, name);
    if (!overloads)
    {
        overloads = safe_malloc(sizeof(DynamicArray));
        array_init(overloads, 2);
        hashtable_put(analyzer->current_scope->symbols, name, overloads);
    }
    return overloads;
}

void semantic_add_global_function_with_params(SemanticAnalyzer *analyzer, Function *func)
{
    if (debug_enabled)
    {
        printf("[DEBUG] Adding function with params: %s (return_type: %d, params: %zu)\n",
               func->name, func->return_type, func->params.size);
    }

    Symbol *symbol = safe_malloc(sizeof(Symbol));
    symbol->name = string_copy(func->name);
    symbol->type = SYMBOL_FUNCTION;
    symbol->data_type = func->return_type;
    symbol->scope_level = analyzer->current_scope->level;

    array_init(&symbol->data.function.params, func->params.size);
    for (size_t j = 0; j < func->params.size; j++)
    {
        Parameter *param = (Parameter *)array_get(&func->params, j);
        Parameter *param_copy = safe_malloc(sizeof(Parameter));
        param_copy->name = string_copy(param->name);
        param_copy->type = param->type;
        array_push(&symbol->data.function.params, param_copy);
    }

    DynamicArray *overloads = get_or_create_overload_set(analyzer, func->name);
    array_push(overloads, symbol);

    if (debug_enabled)
    {
        printf("[DEBUG] Successfully added function %s with %zu parameters\n",
               func->name, func->params.size);
    }
}

void semantic_add_global_symbol(SemanticAnalyzer *analyzer, const char *name, SymbolType type, DataType data_type)
{
    if (type == SYMBOL_FUNCTION)
    {
        Symbol *symbol = scope_define_function(analyzer, name, data_type);

        if (debug_enabled)
        {
            printf("[DEBUG] Added global function: %s (return_type: %d)\n", name, data_type);
        }

        (void)symbol;
    }
    else
    {
        Symbol *symbol = scope_define(analyzer, name, type, data_type);

        if (debug_enabled)
        {
            printf("[DEBUG] Added global symbol: %s (type: %d, data_type: %d)\n",
                   name, type, data_type);
        }

        (void)symbol;
    }
}