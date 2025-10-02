#include "frontend/ast.h"
#include "frontend/astexpr.h"
#include "frontend/aststmt.h"

Function *function_create(const char *name, DataType return_type)
{
    Function *func = safe_malloc(sizeof(Function));
    func->name = string_copy(name);
    func->return_type = return_type;
    array_init(&func->params, 4);
    func->body = NULL;
    return func;
}

Parameter *parameter_create(const char *name, DataType type)
{
    Parameter *param = safe_malloc(sizeof(Parameter));
    param->name = string_copy(name);
    param->type = type;
    return param;
}

Program *program_create(void)
{
    Program *program = safe_malloc(sizeof(Program));
    array_init(&program->functions, 4);
    array_init(&program->includes, 4);
    array_init(&program->ffi_functions, 4);
    return program;
}

void function_add_param(Function *func, Parameter *param)
{
    array_push(&func->params, param);
}

void program_add_function(Program *program, Function *func)
{
    array_push(&program->functions, func);
}

void program_add_include(Program *program, Stmt *include_stmt)
{
    array_push(&program->includes, include_stmt);
}

void program_add_ffi_function(Program *program, FFIFunction *ffi_func)
{
    array_push(&program->ffi_functions, ffi_func);
}

void parameter_destroy(Parameter *param)
{
    if (!param)
        return;
    safe_free(param->name);
    safe_free(param);
}

void function_destroy(Function *func)
{
    if (!func)
        return;
    safe_free(func->name);
    for (size_t i = 0; i < func->params.size; i++)
    {
        parameter_destroy((Parameter *)array_get(&func->params, i));
    }
    array_free(&func->params);
    stmt_destroy(func->body);
    safe_free(func);
}

void program_destroy(Program *program)
{
    if (!program)
        return;
    for (size_t i = 0; i < program->functions.size; i++)
    {
        function_destroy((Function *)array_get(&program->functions, i));
    }
    array_free(&program->functions);

    for (size_t i = 0; i < program->includes.size; i++)
    {
        stmt_destroy((Stmt *)array_get(&program->includes, i));
    }
    array_free(&program->includes);

    for (size_t i = 0; i < program->ffi_functions.size; i++)
    {
        ffi_function_destroy((FFIFunction *)array_get(&program->ffi_functions, i));
    }
    array_free(&program->ffi_functions);

    safe_free(program);
}

static void print_indent(int indent)
{
    for (int i = 0; i < indent; i++)
    {
        printf("  ");
    }
}

void function_print(const Function *func, int indent)
{
    if (!func)
        return;

    print_indent(indent);
    printf("Function: %s -> %s\n", func->name, data_type_to_string(func->return_type));

    print_indent(indent + 1);
    printf("Parameters:\n");
    for (size_t i = 0; i < func->params.size; i++)
    {
        Parameter *param = (Parameter *)array_get(&func->params, i);
        print_indent(indent + 2);
        printf("%s: %s\n", param->name, data_type_to_string(param->type));
    }

    print_indent(indent + 1);
    printf("Body:\n");
    stmt_print(func->body, indent + 2);
}

void program_print(const Program *program)
{
    if (!program)
        return;

    printf("Program:\n");
    for (size_t i = 0; i < program->functions.size; i++)
    {
        function_print((Function *)array_get(&program->functions, i), 1);
    }
}

const char *data_type_to_string(DataType type)
{
    switch (type)
    {
    case TYPE_INT:
        return "int";
    case TYPE_BOOL:
        return "bool";
    case TYPE_VOID:
        return "void";
    case TYPE_ARRAY:
        return "array";
    case TYPE_FLOAT:
        return "float";
    case TYPE_DOUBLE:
        return "double";
    case TYPE_STRING:
        return "string";
    case TYPE_NULL:
        return "null";
    default:
        return "unknown";
    }
}

DataType token_to_data_type(TLTokenType token_type)
{
    switch (token_type)
    {
    case TOKEN_INT:
        return TYPE_INT;
    case TOKEN_BOOL:
        return TYPE_BOOL;
    case TOKEN_FLOAT:
        return TYPE_FLOAT;
    case TOKEN_DOUBLE:
        return TYPE_DOUBLE;
    case TOKEN_STRING_TYPE:
        return TYPE_STRING;
    default:
        return TYPE_VOID;
    }
}