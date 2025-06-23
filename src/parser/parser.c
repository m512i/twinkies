#include "../include/parser.h"

Parser* parser_create(Lexer* lexer, Error* error) {
    Parser* parser = safe_malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->error = error;
    parser->had_error = false;
    parser->panic_mode = false;
    
    parser->current = lexer_next_token(lexer);
    parser->previous = parser->current;
    
    return parser;
}

void parser_destroy(Parser* parser) {
    safe_free(parser);
}

void parser_advance(Parser* parser) {
    parser->previous = parser->current;
    
    while (true) {
        parser->current = lexer_next_token(parser->lexer);
        if (parser->current.type != TOKEN_ERROR) break;
        
        parser_error(parser, parser->current.lexeme);
    }
}

void parser_consume(Parser* parser, TokenType type, const char* message) {
    if (parser->current.type == type) {
        parser_advance(parser);
        return;
    }
    
    parser_error(parser, message);
}

bool parser_check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

bool parser_match(Parser* parser, TokenType type) {
    if (!parser_check(parser, type)) return false;
    parser_advance(parser);
    return true;
}

void parser_synchronize(Parser* parser) {
    parser->panic_mode = false;
    
    while (parser->current.type != TOKEN_EOF) {
        if (parser->previous.type == TOKEN_SEMICOLON) return;
        
        switch (parser->current.type) {
            case TOKEN_FUNC:
            case TOKEN_LET:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_RETURN:
                return;
            default:
                break;
        }
        
        parser_advance(parser);
    }
}

void parser_error(Parser* parser, const char* message) {
    if (parser->panic_mode) return;
    parser->panic_mode = true;
    parser->had_error = true;
    
    if (parser->error) {
        error_set(parser->error, ERROR_PARSER, message, 
                 parser->current.line, parser->current.column);
    }
    
    parser_synchronize(parser);
}

Expr* parse_expression(Parser* parser) {
    return parse_logical_or(parser);
}

Expr* parse_logical_or(Parser* parser) {
    Expr* expr = parse_logical_and(parser);
    
    while (parser_match(parser, TOKEN_OR)) {
        TokenType operator = parser->previous.type;
        Expr* right = parse_logical_and(parser);
        expr = expr_binary(expr, operator, right, expr->line, expr->column);
    }
    
    return expr;
}

Expr* parse_logical_and(Parser* parser) {
    Expr* expr = parse_equality(parser);
    
    while (parser_match(parser, TOKEN_AND)) {
        TokenType operator = parser->previous.type;
        Expr* right = parse_equality(parser);
        expr = expr_binary(expr, operator, right, expr->line, expr->column);
    }
    
    return expr;
}

Expr* parse_equality(Parser* parser) {
    Expr* expr = parse_comparison(parser);
    
    while (parser_match(parser, TOKEN_NE) || parser_match(parser, TOKEN_EQ)) {
        TokenType operator = parser->previous.type;
        Expr* right = parse_comparison(parser);
        expr = expr_binary(expr, operator, right, expr->line, expr->column);
    }
    
    return expr;
}

Expr* parse_comparison(Parser* parser) {
    Expr* expr = parse_term(parser);
    
    while (parser_match(parser, TOKEN_GT) || parser_match(parser, TOKEN_GE) ||
           parser_match(parser, TOKEN_LT) || parser_match(parser, TOKEN_LE)) {
        TokenType operator = parser->previous.type;
        Expr* right = parse_term(parser);
        expr = expr_binary(expr, operator, right, expr->line, expr->column);
    }
    
    return expr;
}

Expr* parse_term(Parser* parser) {
    Expr* expr = parse_factor(parser);
    
    while (parser_match(parser, TOKEN_MINUS) || parser_match(parser, TOKEN_PLUS)) {
        TokenType operator = parser->previous.type;
        Expr* right = parse_factor(parser);
        expr = expr_binary(expr, operator, right, expr->line, expr->column);
    }
    
    return expr;
}

Expr* parse_factor(Parser* parser) {
    Expr* expr = parse_unary(parser);
    
    while (parser_match(parser, TOKEN_SLASH) || parser_match(parser, TOKEN_STAR) ||
           parser_match(parser, TOKEN_PERCENT)) {
        TokenType operator = parser->previous.type;
        Expr* right = parse_unary(parser);
        expr = expr_binary(expr, operator, right, expr->line, expr->column);
    }
    
    return expr;
}

Expr* parse_unary(Parser* parser) {
    if (parser_match(parser, TOKEN_BANG) || parser_match(parser, TOKEN_MINUS)) {
        TokenType operator = parser->previous.type;
        Expr* right = parse_unary(parser);
        return expr_unary(operator, right, parser->previous.line, parser->previous.column);
    }
    
    return parse_primary(parser);
}

Expr* parse_primary(Parser* parser) {
    if (parser_match(parser, TOKEN_NUMBER)) {
        return expr_literal_number(parser->previous.literal.number_value,
                                 parser->previous.line, parser->previous.column);
    }
    
    if (parser_match(parser, TOKEN_IDENTIFIER)) {
        return parse_call(parser);
    }
    
    if (parser_match(parser, TOKEN_LPAREN)) {
        Expr* expr = parse_expression(parser);
        parser_consume(parser, TOKEN_RPAREN, "Expect ')' after expression.");
        return expr_group(expr, expr->line, expr->column);
    }
    
    parser_error(parser, "Expect expression.");
    return NULL;
}

Expr* parse_call(Parser* parser) {
    Expr* expr = expr_variable(parser->previous.lexeme, 
                              parser->previous.line, parser->previous.column);
    
    while (true) {
        if (parser_match(parser, TOKEN_LPAREN)) {
            expr = finish_call(parser, expr);
        } else {
            break;
        }
    }
    
    return expr;
}

Expr* finish_call(Parser* parser, Expr* callee) {
    Expr* call = expr_call(callee->data.variable.name, callee->line, callee->column);
    expr_destroy(callee); 
    
    if (!parser_check(parser, TOKEN_RPAREN)) {
        do {
            if (call->data.call.args.size >= 255) {
                parser_error(parser, "Cannot have more than 255 arguments.");
            }
            expr_add_call_arg(call, parse_expression(parser));
        } while (parser_match(parser, TOKEN_COMMA));
    }
    
    parser_consume(parser, TOKEN_RPAREN, "Expect ')' after arguments.");
    return call;
}

Stmt* parse_statement(Parser* parser) {
    printf("[DEBUG] Entered parse_statement, token: %s\n", token_type_to_string(parser->current.type)); fflush(stdout);
    Stmt* result = NULL;
    
    if (parser_match(parser, TOKEN_LET)) {
        printf("[DEBUG] Parsing var declaration\n"); fflush(stdout);
        result = parse_var_declaration(parser);
    } else if (parser_match(parser, TOKEN_IF)) {
        printf("[DEBUG] Parsing if statement\n"); fflush(stdout);
        result = parse_if_statement(parser);
    } else if (parser_match(parser, TOKEN_WHILE)) {
        printf("[DEBUG] Parsing while statement\n"); fflush(stdout);
        result = parse_while_statement(parser);
    } else if (parser_match(parser, TOKEN_RETURN)) {
        printf("[DEBUG] Parsing return statement\n"); fflush(stdout);
        result = parse_return_statement(parser);
    } else if (parser_match(parser, TOKEN_PRINT)) {
        printf("[DEBUG] Parsing print statement\n"); fflush(stdout);
        result = parse_print_statement(parser);
    } else if (parser_match(parser, TOKEN_LBRACE)) {
        printf("[DEBUG] Parsing block\n"); fflush(stdout);
        result = parse_block(parser);
    } else if (parser_check(parser, TOKEN_IDENTIFIER)) {
        printf("[DEBUG] Parsing assignment or expression statement\n"); fflush(stdout);
        result = parse_assignment(parser);
    } else {
        printf("[DEBUG] Parsing expression statement\n"); fflush(stdout);
        result = parse_expression_statement(parser);
    }
    
    printf("[DEBUG] Exiting parse_statement\n"); fflush(stdout);
    return result;
}

Stmt* parse_var_declaration(Parser* parser) {
    parser_consume(parser, TOKEN_IDENTIFIER, "Expect variable name.");
    char* name = string_copy(parser->previous.lexeme);
    
    parser_consume(parser, TOKEN_COLON, "Expect ':' after variable name.");
    
    DataType type = TYPE_INT; 
    if (parser_match(parser, TOKEN_INT)) {
        type = TYPE_INT;
    } else if (parser_match(parser, TOKEN_BOOL)) {
        type = TYPE_BOOL;
    } else {
        parser_error(parser, "Expect type annotation.");
    }
    
    Expr* initializer = NULL;
    if (parser_match(parser, TOKEN_ASSIGN)) {
        initializer = parse_expression(parser);
    }
    
    parser_consume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    
    return stmt_var_decl(name, type, initializer, parser->previous.line, parser->previous.column);
}

Stmt* parse_assignment(Parser* parser) {
    Expr* expr = parse_expression(parser);
    
    if (parser_match(parser, TOKEN_ASSIGN)) {
        Expr* value = parse_expression(parser);
        
        if (expr->type == EXPR_VARIABLE) {
            char* name = string_copy(expr->data.variable.name);
            expr_destroy(expr);
            parser_consume(parser, TOKEN_SEMICOLON, "Expect ';' after assignment.");
            return stmt_assignment(name, value, parser->previous.line, parser->previous.column);
        }
        
        parser_error(parser, "Invalid assignment target.");
    }
    
    parser_consume(parser, TOKEN_SEMICOLON, "Expect ';' after expression.");
    return stmt_expr(expr, expr->line, expr->column);
}

Stmt* parse_if_statement(Parser* parser) {
    printf("[DEBUG] Entered parse_if_statement\n"); fflush(stdout);
    parser_consume(parser, TOKEN_LPAREN, "Expect '(' after 'if'.");
    printf("[DEBUG] Got LPAREN after if\n"); fflush(stdout);
    Expr* condition = parse_expression(parser);
    printf("[DEBUG] Parsed condition\n"); fflush(stdout);
    parser_consume(parser, TOKEN_RPAREN, "Expect ')' after if condition.");
    printf("[DEBUG] Got RPAREN after condition\n"); fflush(stdout);
    
    Stmt* then_branch;
    if (parser_match(parser, TOKEN_LBRACE)) {
        printf("[DEBUG] Parsing then branch as block\n"); fflush(stdout);
        then_branch = parse_block(parser);
    } else {
        printf("[DEBUG] Parsing then branch as single statement\n"); fflush(stdout);
        then_branch = parse_statement(parser);
    }
    printf("[DEBUG] Parsed then branch\n"); fflush(stdout);
    
    Stmt* else_branch = NULL;
    if (parser_match(parser, TOKEN_ELSE)) {
        printf("[DEBUG] Found else branch\n"); fflush(stdout);
        if (parser_match(parser, TOKEN_LBRACE)) {
            printf("[DEBUG] Parsing else branch as block\n"); fflush(stdout);
            else_branch = parse_block(parser);
        } else {
            printf("[DEBUG] Parsing else branch as single statement\n"); fflush(stdout);
            else_branch = parse_statement(parser);
        }
    }
    
    printf("[DEBUG] Exiting parse_if_statement\n"); fflush(stdout);
    return stmt_if(condition, then_branch, else_branch, 
                  parser->previous.line, parser->previous.column);
}

Stmt* parse_while_statement(Parser* parser) {
    parser_consume(parser, TOKEN_LPAREN, "Expect '(' after 'while'.");
    Expr* condition = parse_expression(parser);
    parser_consume(parser, TOKEN_RPAREN, "Expect ')' after condition.");
    
    Stmt* body = parse_statement(parser);
    
    return stmt_while(condition, body, parser->previous.line, parser->previous.column);
}

Stmt* parse_return_statement(Parser* parser) {
    Expr* value = NULL;
    if (!parser_check(parser, TOKEN_SEMICOLON)) {
        value = parse_expression(parser);
    }
    
    parser_consume(parser, TOKEN_SEMICOLON, "Expect ';' after return value.");
    return stmt_return(value, parser->previous.line, parser->previous.column);
}

Stmt* parse_print_statement(Parser* parser) {
    parser_consume(parser, TOKEN_LPAREN, "Expect '(' after 'print'.");
    Expr* value = parse_expression(parser);
    parser_consume(parser, TOKEN_RPAREN, "Expect ')' after print value.");
    parser_consume(parser, TOKEN_SEMICOLON, "Expect ';' after print statement.");
    
    return stmt_print_stmt(value, parser->previous.line, parser->previous.column);
}

Stmt* parse_expression_statement(Parser* parser) {
    Expr* expr = parse_expression(parser);
    parser_consume(parser, TOKEN_SEMICOLON, "Expect ';' after expression.");
    return stmt_expr(expr, expr->line, expr->column);
}

Stmt* parse_block(Parser* parser) {
    printf("[DEBUG] Entered parse_block\n"); fflush(stdout);
    Stmt* block = stmt_block(parser->previous.line, parser->previous.column);
    
    while (!parser_check(parser, TOKEN_RBRACE) && !parser_check(parser, TOKEN_EOF)) {
        printf("[DEBUG] Parsing statement, current token: %s\n", token_type_to_string(parser->current.type)); fflush(stdout);
        Stmt* stmt = parse_statement(parser);
        if (stmt) {
            stmt_add_block_stmt(block, stmt);
            printf("[DEBUG] Added statement to block\n"); fflush(stdout);
        } else {
            printf("[DEBUG] Failed to parse statement\n"); fflush(stdout);
            break;
        }
    }
    
    printf("[DEBUG] Exiting block, consuming RBRACE\n"); fflush(stdout);
    parser_consume(parser, TOKEN_RBRACE, "Expect '}' after block.");
    printf("[DEBUG] Exiting parse_block\n"); fflush(stdout);
    return block;
}

Function* parse_function(Parser* parser) {
    printf("[DEBUG] Entered parse_function\n"); fflush(stdout);
    parser_consume(parser, TOKEN_IDENTIFIER, "Expect function name.");
    char* name = string_copy(parser->previous.lexeme);
    printf("[DEBUG] Function name: %s\n", name); fflush(stdout);
    
    parser_consume(parser, TOKEN_LPAREN, "Expect '(' after function name.");
    printf("[DEBUG] Got LPAREN\n"); fflush(stdout);
    
    Function* function = function_create(name, TYPE_INT); 
    if (!parser_check(parser, TOKEN_RPAREN)) {
        do {
            if (function->params.size >= 255) {
                parser_error(parser, "Cannot have more than 255 parameters.");
            }
            function_add_param(function, parse_parameter(parser));
        } while (parser_match(parser, TOKEN_COMMA));
    }
    
    parser_consume(parser, TOKEN_RPAREN, "Expect ')' after parameters.");
    printf("[DEBUG] Got RPAREN\n"); fflush(stdout);
    parser_consume(parser, TOKEN_ARROW, "Expect '->' after function parameters.");
    printf("[DEBUG] Got ARROW\n"); fflush(stdout);
    
    if (parser_match(parser, TOKEN_INT)) {
        function->return_type = TYPE_INT;
        printf("[DEBUG] Return type: INT\n"); fflush(stdout);
    } else if (parser_match(parser, TOKEN_BOOL)) {
        function->return_type = TYPE_BOOL;
        printf("[DEBUG] Return type: BOOL\n"); fflush(stdout);
    } else {
        printf("[DEBUG] Expected return type, got: %s\n", token_type_to_string(parser->current.type)); fflush(stdout);
        parser_error(parser, "Expect return type.");
        function_destroy(function);
        return NULL;
    }
    
    parser_consume(parser, TOKEN_LBRACE, "Expect '{' before function body.");
    printf("[DEBUG] Got LBRACE, parsing body\n"); fflush(stdout);
    function->body = parse_block(parser);
    printf("[DEBUG] Finished parsing function body\n"); fflush(stdout);
    
    return function;
}

Parameter* parse_parameter(Parser* parser) {
    parser_consume(parser, TOKEN_IDENTIFIER, "Expect parameter name.");
    char* name = string_copy(parser->previous.lexeme);
    
    parser_consume(parser, TOKEN_COLON, "Expect ':' after parameter name.");
    
    DataType type = TYPE_INT; 
    if (parser_match(parser, TOKEN_INT)) {
        type = TYPE_INT;
    } else if (parser_match(parser, TOKEN_BOOL)) {
        type = TYPE_BOOL;
    } else {
        parser_error(parser, "Expect parameter type.");
    }
    
    return parameter_create(name, type);
}

Program* parser_parse(Parser* parser) {
    printf("[DEBUG] Entered parser_parse\n"); fflush(stdout);
    Program* program = program_create();
    
    while (!parser_check(parser, TOKEN_EOF)) {
        printf("[DEBUG] Current token: %s\n", token_type_to_string(parser->current.type)); fflush(stdout);
        if (parser_match(parser, TOKEN_FUNC)) {
            printf("[DEBUG] Parsing function\n"); fflush(stdout);
            Function* func = parse_function(parser);
            if (func) {
                program_add_function(program, func);
                printf("[DEBUG] Added function to program\n"); fflush(stdout);
            } else {
                printf("[DEBUG] Failed to parse function\n"); fflush(stdout);
                break;
            }
        } else {
            printf("[DEBUG] Expected function declaration, got: %s\n", token_type_to_string(parser->current.type)); fflush(stdout);
            parser_error(parser, "Expect function declaration.");
            break;
        }
    }
    
    printf("[DEBUG] Exiting parser_parse, had_error: %d\n", parser->had_error); fflush(stdout);
    return program;
} 