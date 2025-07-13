#include "frontend/astexpr.h"

Expr *expr_literal_number(int64_t value, int line, int column)
{
    Expr *expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->line = line;
    expr->column = column;
    expr->data.literal.value.number_value = value;
    expr->data.literal.is_bool_literal = false;
    expr->data.literal.is_float_literal = false;
    expr->data.literal.is_string_literal = false;
    return expr;
}

Expr *expr_literal_bool(bool value, int line, int column)
{
    Expr *expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->line = line;
    expr->column = column;
    expr->data.literal.value.bool_value = value;
    expr->data.literal.is_bool_literal = true;
    expr->data.literal.is_float_literal = false;
    return expr;
}

Expr *expr_literal_float(double value, int line, int column)
{
    Expr *expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->line = line;
    expr->column = column;
    expr->data.literal.value.float_value = value;
    expr->data.literal.is_bool_literal = false;
    expr->data.literal.is_float_literal = true;
    expr->data.literal.is_string_literal = false;
    return expr;
}

Expr *expr_literal_string(const char *value, int line, int column)
{
    Expr *expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->line = line;
    expr->column = column;
    expr->data.literal.value.string_value = string_copy(value);
    expr->data.literal.is_bool_literal = false;
    expr->data.literal.is_float_literal = false;
    expr->data.literal.is_string_literal = true;
    return expr;
}

Expr *expr_literal_null(int line, int column)
{
    Expr *expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_NULL_LITERAL;
    expr->line = line;
    expr->column = column;
    return expr;
}

Expr *expr_variable(const char *name, int line, int column)
{
    Expr *expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_VARIABLE;
    expr->line = line;
    expr->column = column;
    expr->data.variable.name = string_copy(name);
    return expr;
}

Expr *expr_binary(Expr *left, TLTokenType op, Expr *right, int line, int column)
{
    Expr *expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_BINARY;
    expr->line = line;
    expr->column = column;
    expr->data.binary.left = left;
    expr->data.binary.operator= op;
    expr->data.binary.right = right;
    return expr;
}

Expr *expr_unary(TLTokenType op, Expr *operand, int line, int column)
{
    Expr *expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_UNARY;
    expr->line = line;
    expr->column = column;
    expr->data.unary.operator= op;
    expr->data.unary.operand = operand;
    return expr;
}

Expr *expr_call(const char *name, int line, int column)
{
    Expr *expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_CALL;
    expr->line = line;
    expr->column = column;
    expr->data.call.name = string_copy(name);
    array_init(&expr->data.call.args, 4);
    return expr;
}

Expr *expr_group(Expr *expression, int line, int column)
{
    Expr *expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_GROUP;
    expr->line = line;
    expr->column = column;
    expr->data.group.expression = expression;
    return expr;
}

Expr *expr_array_index(Expr *array, Expr *index, int line, int column)
{
    Expr *expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_ARRAY_INDEX;
    expr->line = line;
    expr->column = column;
    expr->data.array_index.array = array;
    expr->data.array_index.index = index;
    return expr;
}

Expr *expr_string_index(Expr *string, Expr *index, int line, int column)
{
    Expr *expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_STRING_INDEX;
    expr->line = line;
    expr->column = column;
    expr->data.string_index.string = string;
    expr->data.string_index.index = index;
    return expr;
}

void expr_add_call_arg(Expr *call, Expr *arg)
{
    array_push(&call->data.call.args, arg);
}

void expr_destroy(Expr *expr)
{
    if (!expr)
        return;

    switch (expr->type)
    {
    case EXPR_LITERAL:
        if (expr->data.literal.is_string_literal)
        {
            safe_free(expr->data.literal.value.string_value);
        }
        break;
    case EXPR_VARIABLE:
        safe_free(expr->data.variable.name);
        break;
    case EXPR_BINARY:
        expr_destroy(expr->data.binary.left);
        expr_destroy(expr->data.binary.right);
        break;
    case EXPR_UNARY:
        expr_destroy(expr->data.unary.operand);
        break;
    case EXPR_CALL:
        safe_free(expr->data.call.name);
        for (size_t i = 0; i < expr->data.call.args.size; i++)
        {
            Expr *arg = (Expr *)array_get(&expr->data.call.args, i);
            expr_destroy(arg);
        }
        array_free(&expr->data.call.args);
        break;
    case EXPR_GROUP:
        expr_destroy(expr->data.group.expression);
        break;
    case EXPR_ARRAY_INDEX:
        expr_destroy(expr->data.array_index.array);
        expr_destroy(expr->data.array_index.index);
        break;
    case EXPR_STRING_INDEX:
        expr_destroy(expr->data.string_index.string);
        expr_destroy(expr->data.string_index.index);
        break;
    case EXPR_NULL_LITERAL:
        break;
    }

    safe_free(expr);
}

Expr *expr_copy(Expr *expr)
{
    if (!expr)
        return NULL;

    Expr *copy = safe_malloc(sizeof(Expr));
    copy->type = expr->type;
    copy->line = expr->line;
    copy->column = expr->column;

    switch (expr->type)
    {
    case EXPR_LITERAL:
        copy->data.literal.is_string_literal = expr->data.literal.is_string_literal;
        copy->data.literal.is_float_literal = expr->data.literal.is_float_literal;
        if (expr->data.literal.is_string_literal)
        {
            copy->data.literal.value.string_value = string_copy(expr->data.literal.value.string_value);
        }
        else if (expr->data.literal.is_float_literal)
        {
            copy->data.literal.value.float_value = expr->data.literal.value.float_value;
        }
        else
        {
            copy->data.literal.value.number_value = expr->data.literal.value.number_value;
        }
        break;
    case EXPR_VARIABLE:
        copy->data.variable.name = string_copy(expr->data.variable.name);
        break;
    case EXPR_BINARY:
        copy->data.binary.left = expr_copy(expr->data.binary.left);
        copy->data.binary.operator= expr->data.binary.operator;
        copy->data.binary.right = expr_copy(expr->data.binary.right);
        break;
    case EXPR_UNARY:
        copy->data.unary.operator= expr->data.unary.operator;
        copy->data.unary.operand = expr_copy(expr->data.unary.operand);
        break;
    case EXPR_CALL:
        copy->data.call.name = string_copy(expr->data.call.name);
        array_init(&copy->data.call.args, expr->data.call.args.size);
        for (size_t i = 0; i < expr->data.call.args.size; i++)
        {
            Expr *arg = (Expr *)array_get(&expr->data.call.args, i);
            array_push(&copy->data.call.args, expr_copy(arg));
        }
        break;
    case EXPR_GROUP:
        copy->data.group.expression = expr_copy(expr->data.group.expression);
        break;
    case EXPR_ARRAY_INDEX:
        copy->data.array_index.array = expr_copy(expr->data.array_index.array);
        copy->data.array_index.index = expr_copy(expr->data.array_index.index);
        break;
    case EXPR_STRING_INDEX:
        copy->data.string_index.string = expr_copy(expr->data.string_index.string);
        copy->data.string_index.index = expr_copy(expr->data.string_index.index);
        break;
    case EXPR_NULL_LITERAL:
        break;
    }

    return copy;
}

static void print_indent(int indent)
{
    for (int i = 0; i < indent; i++)
    {
        printf("  ");
    }
}

void expr_print(const Expr *expr, int indent)
{
    if (!expr)
        return;

    print_indent(indent);
    printf("Expr(");

    switch (expr->type)
    {
    case EXPR_LITERAL:
        if (expr->data.literal.is_string_literal)
        {
            printf("StringLiteral: \"%s\"", expr->data.literal.value.string_value);
        }
        else if (expr->data.literal.is_bool_literal)
        {
            printf("BoolLiteral: %s", expr->data.literal.value.bool_value ? "true" : "false");
        }
        else if (expr->data.literal.is_float_literal)
        {
            printf("FloatLiteral: %f", expr->data.literal.value.float_value);
        }
        else
        {
            printf("NumberLiteral: %lld", expr->data.literal.value.number_value);
        }
        break;
    case EXPR_VARIABLE:
        printf("Variable: %s", expr->data.variable.name);
        break;
    case EXPR_BINARY:
        printf("Binary: %s\n", token_type_to_string(expr->data.binary.operator));
        expr_print(expr->data.binary.left, indent + 1);
        expr_print(expr->data.binary.right, indent + 1);
        break;
    case EXPR_UNARY:
        printf("Unary: %s\n", token_type_to_string(expr->data.unary.operator));
        expr_print(expr->data.unary.operand, indent + 1);
        break;
    case EXPR_CALL:
        printf("Call: %s", expr->data.call.name);
        for (size_t i = 0; i < expr->data.call.args.size; i++)
        {
            printf("\n");
            expr_print((Expr *)array_get(&expr->data.call.args, i), indent + 1);
        }
        break;
    case EXPR_GROUP:
        printf("Group:\n");
        expr_print(expr->data.group.expression, indent + 1);
        break;
    case EXPR_ARRAY_INDEX:
        printf("ArrayIndex:\n");
        expr_print(expr->data.array_index.array, indent + 1);
        expr_print(expr->data.array_index.index, indent + 1);
        break;
    case EXPR_STRING_INDEX:
        printf("StringIndex:\n");
        expr_print(expr->data.string_index.string, indent + 1);
        expr_print(expr->data.string_index.index, indent + 1);
        break;
    case EXPR_NULL_LITERAL:
        printf("NullLiteral: null");
        break;
    }

    printf(")\n");
}