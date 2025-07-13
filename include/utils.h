#ifndef UTILS_H
#define UTILS_H

#include "common.h"
#include "frontend/ast.h"

char *read_file(const char *filename);

bool compile_file(const char *input_filename, const char *output_filename, bool verbose, bool assembly_output);

void print_tokens(const char *source, const char *filename);
void print_ast(const char *source, const char *filename);
void print_ir(const char *source, const char *filename);
void dump_ast_json(const char *source, const char *filename);
void dump_stmt_json(Stmt *stmt, int indent);
void dump_expr_json(Expr *expr, int indent);
void print_json_indent(int indent);

#endif