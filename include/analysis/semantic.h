#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "common.h"
#include "frontend/ast.h"

typedef enum
{
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_PARAMETER
} SymbolType;

typedef struct
{
    char *name;
    SymbolType type;
    DataType data_type;
    int scope_level;
    int array_size;
    DataType element_type; // For arrays: the element type
    bool is_used;
    bool is_defined;
    int definition_line;
    int definition_column;
    union
    {
        struct
        {
            int param_index;
        } parameter;
        struct
        {
            DynamicArray params;
        } function;
    } data;
} Symbol;

typedef struct Scope
{
    HashTable *symbols;
    struct Scope *parent;
    int level;
} Scope;

typedef struct
{
    Program *program;
    Scope *current_scope;
    ErrorContext *error_context;
    bool had_error;
} SemanticAnalyzer;

SemanticAnalyzer *semantic_create(Program *program, ErrorContext *error_context);
void semantic_destroy(SemanticAnalyzer *analyzer);
bool semantic_analyze(SemanticAnalyzer *analyzer);

Scope *scope_create(Scope *parent);
void scope_destroy(Scope *scope);
void scope_enter(SemanticAnalyzer *analyzer);
void scope_exit(SemanticAnalyzer *analyzer);
Symbol *scope_define(SemanticAnalyzer *analyzer, const char *name, SymbolType type, DataType data_type);
Symbol *scope_define_array(SemanticAnalyzer *analyzer, const char *name, DataType element_type, int size);
Symbol *scope_define_function(SemanticAnalyzer *analyzer, const char *name, DataType return_type);
Symbol *scope_define_ffi_function(SemanticAnalyzer *analyzer, FFIFunction *ffi_func);
Symbol *scope_resolve(SemanticAnalyzer *analyzer, const char *name);
int get_array_size(SemanticAnalyzer *analyzer, const char *name);

DataType type_check_expression(SemanticAnalyzer *analyzer, Expr *expr);
DataType type_check_statement(SemanticAnalyzer *analyzer, Stmt *stmt);
DataType type_check_function(SemanticAnalyzer *analyzer, Function *func);
bool type_check_assignment(SemanticAnalyzer *analyzer, DataType target_type, DataType value_type);
bool type_check_binary(SemanticAnalyzer *analyzer, TLTokenType operator, DataType left_type, DataType right_type, int line, int column);
bool type_check_unary(SemanticAnalyzer *analyzer, TLTokenType operator, DataType operand_type, int line, int column);

void semantic_error(SemanticAnalyzer *analyzer, const char *message, int line, int column);
void semantic_error_with_suggestion(SemanticAnalyzer *analyzer, const char *message, const char *suggestion, int line, int column);
void semantic_error_type_mismatch(SemanticAnalyzer *analyzer, DataType expected, DataType actual, int line, int column);
void semantic_error_undefined(SemanticAnalyzer *analyzer, const char *name, int line, int column);
void semantic_error_redefined(SemanticAnalyzer *analyzer, const char *name, int line, int column);
void semantic_error_array_bounds(SemanticAnalyzer *analyzer, const char *array_name, int index, int size, int line, int column);
void semantic_error_invalid_operation(SemanticAnalyzer *analyzer, const char *operation, DataType type, int line, int column);

void semantic_warning(SemanticAnalyzer *analyzer, const char *message, int line, int column);
void semantic_warning_with_suggestion(SemanticAnalyzer *analyzer, const char *message, const char *suggestion, int line, int column);
void semantic_warning_unused_variable(SemanticAnalyzer *analyzer, const char *name, int line, int column);
void semantic_warning_unreachable_code(SemanticAnalyzer *analyzer, int line, int column);
void semantic_warning_type_conversion(SemanticAnalyzer *analyzer, DataType from_type, DataType to_type, int line, int column);
void semantic_warning_performance(SemanticAnalyzer *analyzer, const char *issue, int line, int column);

const char *symbol_type_to_string(SymbolType type);
bool is_numeric_type(DataType type);
bool is_boolean_type(DataType type);
bool types_are_compatible(DataType type1, DataType type2);

#endif