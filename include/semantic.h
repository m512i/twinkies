#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "common.h"
#include "ast.h"

typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_PARAMETER
} SymbolType;

typedef struct {
    char* name;
    SymbolType type;
    DataType data_type;
    int scope_level;
    union {
        struct {
            int param_index;
        } parameter;
        struct {
            DynamicArray params; 
        } function;
    } data;
} Symbol;

typedef struct Scope {
    HashTable* symbols;
    struct Scope* parent;
    int level;
} Scope;

typedef struct {
    Program* program;
    Scope* current_scope;
    Error* error;
    bool had_error;
} SemanticAnalyzer;

SemanticAnalyzer* semantic_create(Program* program, Error* error);
void semantic_destroy(SemanticAnalyzer* analyzer);
bool semantic_analyze(SemanticAnalyzer* analyzer);

Scope* scope_create(Scope* parent);
void scope_destroy(Scope* scope);
void scope_enter(SemanticAnalyzer* analyzer);
void scope_exit(SemanticAnalyzer* analyzer);
Symbol* scope_define(SemanticAnalyzer* analyzer, const char* name, SymbolType type, DataType data_type);
Symbol* scope_resolve(SemanticAnalyzer* analyzer, const char* name);

DataType type_check_expression(SemanticAnalyzer* analyzer, Expr* expr);
DataType type_check_statement(SemanticAnalyzer* analyzer, Stmt* stmt);
DataType type_check_function(SemanticAnalyzer* analyzer, Function* func);
bool type_check_assignment(SemanticAnalyzer* analyzer, DataType target_type, DataType value_type);
bool type_check_binary(SemanticAnalyzer* analyzer, TokenType operator, DataType left_type, DataType right_type);
bool type_check_unary(SemanticAnalyzer* analyzer, TokenType operator, DataType operand_type);

void semantic_error(SemanticAnalyzer* analyzer, const char* message, int line, int column);
void semantic_error_type_mismatch(SemanticAnalyzer* analyzer, DataType expected, DataType actual, int line, int column);
void semantic_error_undefined(SemanticAnalyzer* analyzer, const char* name, int line, int column);
void semantic_error_redefined(SemanticAnalyzer* analyzer, const char* name, int line, int column);

const char* symbol_type_to_string(SymbolType type);
bool is_numeric_type(DataType type);
bool is_boolean_type(DataType type);
bool types_are_compatible(DataType type1, DataType type2);

#endif 