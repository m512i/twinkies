#ifndef ASTSTMT_H
#define ASTSTMT_H

#include "common/common.h"
#include "frontend/ast/ast.h"

Stmt *stmt_expr(Expr *expression, int line, int column);
Stmt *stmt_var_decl(const char *name, DataType type, Expr *initializer, int line, int column);
Stmt *stmt_array_decl(const char *name, DataType element_type, int size, Expr *initializer, int line, int column);
Stmt *stmt_assignment(const char *name, Expr *value, int line, int column);
Stmt *stmt_array_assignment(Expr *array, Expr *index, Expr *value, int line, int column);
Stmt *stmt_if(Expr *condition, Stmt *then_branch, Stmt *else_branch, int line, int column);
Stmt *stmt_while(Expr *condition, Stmt *body, int line, int column);
Stmt *stmt_break(int line, int column);
Stmt *stmt_continue(int line, int column);
Stmt *stmt_return(Expr *value, int line, int column);
Stmt *stmt_print_stmt(int line, int column);
Stmt *stmt_include(const char *path, IncludeType type, int line, int column);
Stmt *stmt_block(int line, int column);

void stmt_add_block_stmt(Stmt *block, Stmt *stmt);
void stmt_add_print_arg(Stmt *print_stmt, Expr *arg);
void stmt_destroy(Stmt *stmt);
Stmt *stmt_copy(Stmt *stmt);
void stmt_print(const Stmt *stmt, int indent);

#endif