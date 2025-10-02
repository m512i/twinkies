#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "lexer.h"
#include "ast.h"

typedef struct
{
    Lexer *lexer;
    Token current;
    Token previous;
    ErrorContext *error_context;
    bool had_error;
    bool panic_mode;
    int consecutive_errors;
} Parser;

Parser *parser_create(Lexer *lexer, ErrorContext *error_context);
void parser_destroy(Parser *parser);
Program *parser_parse(Parser *parser);

Expr *parse_expression(Parser *parser);
Expr *parse_logical_or(Parser *parser);
Expr *parse_logical_and(Parser *parser);
Expr *parse_equality(Parser *parser);
Expr *parse_comparison(Parser *parser);
Expr *parse_term(Parser *parser);
Expr *parse_factor(Parser *parser);
Expr *parse_unary(Parser *parser);
Expr *parse_primary(Parser *parser);
Expr *parse_call(Parser *parser);
Expr *finish_call(Parser *parser, Expr *callee);

Stmt *parse_statement(Parser *parser);
Stmt *parse_var_declaration(Parser *parser);
Stmt *parse_assignment(Parser *parser);
Stmt *parse_if_statement(Parser *parser);
Stmt *parse_while_statement(Parser *parser);
Stmt *parse_break_statement(Parser *parser);
Stmt *parse_continue_statement(Parser *parser);
Stmt *parse_return_statement(Parser *parser);
Stmt *parse_print_statement(Parser *parser);
Stmt *parse_include_directive(Parser *parser);
Stmt *parse_expression_statement(Parser *parser);
Stmt *parse_block(Parser *parser);

Function *parse_function(Parser *parser);
Function *parse_function_declaration(Parser *parser);
Parameter *parse_parameter(Parser *parser);

void parser_advance(Parser *parser);
void parser_consume(Parser *parser, TLTokenType type, const char *message);
bool parser_check(Parser *parser, TLTokenType type);
bool parser_match(Parser *parser, TLTokenType type);
void parser_synchronize(Parser *parser);
void parser_error(Parser *parser, const char *message);
void parser_reset_error_count(Parser *parser);

#endif