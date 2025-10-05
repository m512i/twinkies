#ifndef AST_H
#define AST_H

#include "common/common.h"
#include "frontend/lexer/lexer.h"
#include "modules/ffi/fficonfig.h"

typedef enum
{
    INCLUDE_SYSTEM,
    INCLUDE_LOCAL
} IncludeType;

typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct Function Function;
typedef struct Program Program;
typedef struct Parameter Parameter;

typedef enum
{
    EXPR_LITERAL,
    EXPR_VARIABLE,
    EXPR_BINARY,
    EXPR_UNARY,
    EXPR_CALL,
    EXPR_GROUP,
    EXPR_ARRAY_INDEX,
    EXPR_STRING_INDEX,
    EXPR_NULL_LITERAL
} ExprType;

typedef enum
{
    STMT_EXPR,
    STMT_VAR_DECL,
    STMT_ARRAY_DECL,
    STMT_ASSIGNMENT,
    STMT_ARRAY_ASSIGNMENT,
    STMT_IF,
    STMT_WHILE,
    STMT_BREAK,
    STMT_CONTINUE,
    STMT_RETURN,
    STMT_PRINT,
    STMT_BLOCK,
    STMT_INCLUDE
} StmtType;

typedef enum
{
    TYPE_INT,
    TYPE_BOOL,
    TYPE_VOID,
    TYPE_ARRAY,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_NULL
} DataType;

typedef struct
{
    DataType element_type;
    int size; // -1 for dynamic arrays
} ArrayType;

struct Expr
{
    ExprType type;
    int line;
    int column;
    union
    {
        struct
        {
            union
            {
                int64_t number_value;
                bool bool_value;
                double float_value;
                char *string_value;
            } value;
            bool is_bool_literal;
            bool is_float_literal;
            bool is_string_literal;
        } literal;

        struct
        {
            char *name;
        } variable;

        struct
        {
            Expr *left;
            TLTokenType operator;
            Expr *right;
        } binary;

        struct
        {
            TLTokenType operator;
            Expr *operand;
        } unary;

        struct
        {
            char *name;
            DynamicArray args;
        } call;

        struct
        {
            Expr *expression;
        } group;

        struct
        {
            Expr *array;
            Expr *index;
        } array_index;

        struct
        {
            Expr *string;
            Expr *index;
        } string_index;
    } data;
};

struct Stmt
{
    StmtType type;
    int line;
    int column;
    union
    {
        struct
        {
            Expr *expression;
        } expr;

        struct
        {
            char *name;
            DataType type;
            Expr *initializer;
        } var_decl;

        struct
        {
            char *name;
            DataType element_type;
            int size;
            Expr *initializer;
        } array_decl;

        struct
        {
            char *name;
            Expr *value;
        } assignment;

        struct
        {
            Expr *array;
            Expr *index;
            Expr *value;
        } array_assignment;

        struct
        {
            Expr *condition;
            Stmt *then_branch;
            Stmt *else_branch;
        } if_stmt;

        struct
        {
            Expr *condition;
            Stmt *body;
        } while_stmt;

        struct
        {
            Expr *value;
        } return_stmt;

        struct
        {
            DynamicArray args;
        } print_stmt;

        struct
        {
            DynamicArray statements;
        } block;

        struct
        {
            char *path;
            IncludeType type;
        } include;
    } data;
};

struct Parameter
{
    char *name;
    DataType type;
};

struct Function
{
    char *name;
    DynamicArray params;
    DataType return_type;
    Stmt *body;
};

struct Program
{
    DynamicArray functions;
    DynamicArray includes;
    DynamicArray ffi_functions;
};

Function *function_create(const char *name, DataType return_type);
Parameter *parameter_create(const char *name, DataType type);
Program *program_create(void);

void function_add_param(Function *func, Parameter *param);
void program_add_function(Program *program, Function *func);
void program_add_include(Program *program, Stmt *include_stmt);
void program_add_ffi_function(Program *program, FFIFunction *ffi_func);

void parameter_destroy(Parameter *param);
void function_destroy(Function *func);
void program_destroy(Program *program);

void function_print(const Function *func, int indent);
void program_print(const Program *program);

const char *data_type_to_string(DataType type);
DataType token_to_data_type(TLTokenType token_type);

#endif