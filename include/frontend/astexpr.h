#ifndef ASTEXPR_H
#define ASTEXPR_H

#include "common.h"
#include "ast.h"

Expr *expr_literal_number(int64_t value, int line, int column);
Expr *expr_literal_bool(bool value, int line, int column);
Expr *expr_literal_float(double value, int line, int column);
Expr *expr_literal_string(const char *value, int line, int column);
Expr *expr_literal_null(int line, int column);
Expr *expr_variable(const char *name, int line, int column);
Expr *expr_binary(Expr *left, TLTokenType op, Expr *right, int line, int column);
Expr *expr_unary(TLTokenType op, Expr *operand, int line, int column);
Expr *expr_call(const char *name, int line, int column);
Expr *expr_group(Expr *expression, int line, int column);
Expr *expr_array_index(Expr *array, Expr *index, int line, int column);
Expr *expr_string_index(Expr *string, Expr *index, int line, int column);

void expr_add_call_arg(Expr *call, Expr *arg);
void expr_destroy(Expr *expr);
Expr *expr_copy(Expr *expr);
void expr_print(const Expr *expr, int indent);

#endif