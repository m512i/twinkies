#ifndef AST_H
#define AST_H

#include "common.h"
#include "lexer.h"

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
    EXPR_ARRAY_INDEX
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
    STMT_RETURN,
    STMT_PRINT,
    STMT_BLOCK
} StmtType;

typedef enum
{
    TYPE_INT,
    TYPE_BOOL,
    TYPE_VOID,
    TYPE_ARRAY,
    TYPE_FLOAT,
    TYPE_DOUBLE,
    TYPE_STRING
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
            Expr *value;
        } print_stmt;

        struct
        {
            DynamicArray statements;
        } block;
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
};

Expr *expr_literal_number(int64_t value, int line, int column);
Expr *expr_literal_bool(bool value, int line, int column);
Expr *expr_literal_float(double value, int line, int column);
Expr *expr_literal_string(const char *value, int line, int column);
Expr *expr_variable(const char *name, int line, int column);
Expr *expr_binary(Expr *left, TLTokenType operator, Expr * right, int line, int column);
Expr *expr_unary(TLTokenType operator, Expr * operand, int line, int column);
Expr *expr_call(const char *name, int line, int column);
Expr *expr_group(Expr *expression, int line, int column);
Expr *expr_array_index(Expr *array, Expr *index, int line, int column);

Stmt *stmt_expr(Expr *expression, int line, int column);
Stmt *stmt_var_decl(const char *name, DataType type, Expr *initializer, int line, int column);
Stmt *stmt_array_decl(const char *name, DataType element_type, int size, Expr *initializer, int line, int column);
Stmt *stmt_assignment(const char *name, Expr *value, int line, int column);
Stmt *stmt_array_assignment(Expr *array, Expr *index, Expr *value, int line, int column);
Stmt *stmt_if(Expr *condition, Stmt *then_branch, Stmt *else_branch, int line, int column);
Stmt *stmt_while(Expr *condition, Stmt *body, int line, int column);
Stmt *stmt_return(Expr *value, int line, int column);
Stmt *stmt_print_stmt(Expr *value, int line, int column);
Stmt *stmt_block(int line, int column);

Function *function_create(const char *name, DataType return_type);
Parameter *parameter_create(const char *name, DataType type);
Program *program_create(void);

void expr_add_call_arg(Expr *call, Expr *arg);
void stmt_add_block_stmt(Stmt *block, Stmt *stmt);
void function_add_param(Function *func, Parameter *param);
void program_add_function(Program *program, Function *func);

void expr_destroy(Expr *expr);
void stmt_destroy(Stmt *stmt);
void parameter_destroy(Parameter *param);
void function_destroy(Function *func);
void program_destroy(Program *program);

void expr_print(const Expr *expr, int indent);
void stmt_print(const Stmt *stmt, int indent);
void function_print(const Function *func, int indent);
void program_print(const Program *program);

const char *data_type_to_string(DataType type);
DataType token_to_data_type(TLTokenType token_type);

#endif