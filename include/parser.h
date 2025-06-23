#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer* lexer;
    Token current;
    Token previous;
    Error* error;
    bool had_error;
    bool panic_mode;
} Parser;

Parser* parser_create(Lexer* lexer, Error* error);
void parser_destroy(Parser* parser);
Program* parser_parse(Parser* parser);

Expr* parse_expression(Parser* parser);
Expr* parse_logical_or(Parser* parser);
Expr* parse_logical_and(Parser* parser);
Expr* parse_equality(Parser* parser);
Expr* parse_comparison(Parser* parser);
Expr* parse_term(Parser* parser);
Expr* parse_factor(Parser* parser);
Expr* parse_unary(Parser* parser);
Expr* parse_primary(Parser* parser);
Expr* parse_call(Parser* parser);
Expr* finish_call(Parser* parser, Expr* callee);

Stmt* parse_statement(Parser* parser);
Stmt* parse_var_declaration(Parser* parser);
Stmt* parse_assignment(Parser* parser);
Stmt* parse_if_statement(Parser* parser);
Stmt* parse_while_statement(Parser* parser);
Stmt* parse_return_statement(Parser* parser);
Stmt* parse_print_statement(Parser* parser);
Stmt* parse_expression_statement(Parser* parser);
Stmt* parse_block(Parser* parser);

Function* parse_function(Parser* parser);
Parameter* parse_parameter(Parser* parser);

void parser_advance(Parser* parser);
void parser_consume(Parser* parser, TokenType type, const char* message);
bool parser_check(Parser* parser, TokenType type);
bool parser_match(Parser* parser, TokenType type);
void parser_synchronize(Parser* parser);
void parser_error(Parser* parser, const char* message);

#endif 