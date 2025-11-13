#include "frontend/parser/parser.h"
#include "frontend/ast/astexpr.h"
#include "frontend/ast/aststmt.h"
extern bool debug_enabled;

Expr *finish_call(Parser *parser, Expr *callee);
Expr *finish_array_index(Parser *parser, Expr *array);
Expr *finish_string_index(Parser *parser, Expr *string);
FFIFunction *parse_extern_declaration(Parser *parser, Program *program);

Parser *parser_create(Lexer *lexer, ErrorContext *error_context)
{
    Parser *parser = safe_malloc(sizeof(Parser));
    parser->lexer = lexer;
    parser->error_context = error_context;
    parser->had_error = false;
    parser->panic_mode = false;
    parser->consecutive_errors = 0;

    parser->current = lexer_next_token(lexer);
    parser->previous = parser->current;

    return parser;
}

void parser_destroy(Parser *parser)
{
    safe_free(parser);
}

void parser_advance(Parser *parser)
{
    parser->previous = parser->current;

    while (true)
    {
        parser->current = lexer_next_token(parser->lexer);
        if (parser->current.type != TOKEN_ERROR)
            break;

        parser_error(parser, parser->current.lexeme);
    }
}

void parser_consume(Parser *parser, TLTokenType type, const char *message)
{
    if (parser->current.type == type)
    {
        parser_advance(parser);
        return;
    }

    parser_error(parser, message);
}

bool parser_check(Parser *parser, TLTokenType type)
{
    return parser->current.type == type;
}

bool parser_match(Parser *parser, TLTokenType type)
{
    if (!parser_check(parser, type))
        return false;
    parser_advance(parser);
    return true;
}

void parser_synchronize(Parser *parser)
{
    parser->panic_mode = false;
    parser->consecutive_errors = 0;

    while (parser->current.type != TOKEN_EOF)
    {
        if (parser->previous.type == TOKEN_SEMICOLON)
            return;

        switch (parser->current.type)
        {
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

void parser_reset_error_count(Parser *parser)
{
    parser->consecutive_errors = 0;
}

void parser_error(Parser *parser, const char *message)
{
    if (parser->panic_mode)
        return;

    parser->consecutive_errors++;

    if (parser->consecutive_errors > 10)
    {
        parser->panic_mode = true;
        parser->consecutive_errors = 0;
    }

    parser->had_error = true;
    
    if (debug_enabled)
    {
        printf("[DEBUG] Parser error at line %d, column %d: %s\n", 
               parser->current.line, parser->current.column, message);
        fflush(stdout);
    }

    if (parser->error_context)
    {
        char suggestion[256] = "";

        if (strstr(message, "Expect ')'"))
        {
            strcpy(suggestion, "Check for matching parentheses and ensure all '(' have corresponding ')'");
        }
        else if (strstr(message, "Expect '}'"))
        {
            strcpy(suggestion, "Check for matching braces and ensure all '{' have corresponding '}'");
        }
        else if (strstr(message, "Expect ';'"))
        {
            strcpy(suggestion, "Add semicolon at the end of the statement");
        }
        else if (strstr(message, "Expect expression"))
        {
            strcpy(suggestion, "Provide a valid expression (number, variable, function call, etc.)");
        }
        else if (strstr(message, "Expect variable name"))
        {
            strcpy(suggestion, "Use a valid identifier (letters, digits, underscore, starting with letter)");
        }
        else if (strstr(message, "Expect type annotation"))
        {
            strcpy(suggestion, "Specify the type after colon (e.g., ': int', ': bool')");
        }
        else if (strstr(message, "Expect function declaration"))
        {
            strcpy(suggestion, "Start with 'func' keyword followed by function name and parameters");
        }
        else if (strstr(message, "Expect array size"))
        {
            strcpy(suggestion, "Provide a numeric size for the array (e.g., '[5]')");
        }

        int error_line = parser->current.line;
        int error_column = parser->current.column;
        if (strstr(message, "Expect ';'"))
        {
            error_line = parser->previous.line;
            error_column = parser->previous.column;
        }

        error_context_add_error(parser->error_context, ERROR_PARSER, SEVERITY_ERROR,
                                message, suggestion[0] != '\0' ? suggestion : NULL,
                                error_line, error_column);
    }

    // Don't synchronize immediately - let the parser continue to collect more errors
    // Only synchronize if we're in panic mode
    if (parser->panic_mode)
    {
        parser_synchronize(parser);
        parser->consecutive_errors = 0;
    }
}

Expr *parse_expression(Parser *parser)
{
    return parse_logical_or(parser);
}

Expr *parse_logical_or(Parser *parser)
{
    Expr *expr = parse_logical_and(parser);

    while (parser_match(parser, TOKEN_OR))
    {
        TLTokenType operator= parser->previous.type;
        Expr *right = parse_logical_and(parser);
        expr = expr_binary(expr, operator, right, expr->line, expr->column);
    }

    return expr;
}

Expr *parse_logical_and(Parser *parser)
{
    Expr *expr = parse_equality(parser);

    while (parser_match(parser, TOKEN_AND))
    {
        TLTokenType operator= parser->previous.type;
        Expr *right = parse_equality(parser);
        expr = expr_binary(expr, operator, right, expr->line, expr->column);
    }

    return expr;
}

Expr *parse_equality(Parser *parser)
{
    Expr *expr = parse_comparison(parser);

    while (parser_match(parser, TOKEN_NE) || parser_match(parser, TOKEN_EQ))
    {
        TLTokenType operator= parser->previous.type;
        Expr *right = parse_comparison(parser);
        expr = expr_binary(expr, operator, right, expr->line, expr->column);
    }

    return expr;
}

Expr *parse_comparison(Parser *parser)
{
    Expr *expr = parse_term(parser);

    while (parser_match(parser, TOKEN_GT) || parser_match(parser, TOKEN_GE) ||
           parser_match(parser, TOKEN_LT) || parser_match(parser, TOKEN_LE))
    {
        TLTokenType operator= parser->previous.type;
        Expr *right = parse_term(parser);
        expr = expr_binary(expr, operator, right, expr->line, expr->column);
    }

    return expr;
}

Expr *parse_term(Parser *parser)
{
    Expr *expr = parse_factor(parser);

    while (parser_match(parser, TOKEN_MINUS) || parser_match(parser, TOKEN_PLUS))
    {
        TLTokenType operator= parser->previous.type;
        Expr *right = parse_factor(parser);
        expr = expr_binary(expr, operator, right, expr->line, expr->column);
    }

    return expr;
}

Expr *parse_factor(Parser *parser)
{
    Expr *expr = parse_unary(parser);

    while (parser_match(parser, TOKEN_SLASH) || parser_match(parser, TOKEN_STAR) ||
           parser_match(parser, TOKEN_PERCENT))
    {
        TLTokenType operator= parser->previous.type;
        Expr *right = parse_unary(parser);
        expr = expr_binary(expr, operator, right, expr->line, expr->column);
    }

    return expr;
}

Expr *parse_unary(Parser *parser)
{
    if (parser_match(parser, TOKEN_BANG) || parser_match(parser, TOKEN_MINUS))
    {
        TLTokenType operator= parser->previous.type;
        Expr *right = parse_unary(parser);
        return expr_unary(operator, right, parser->previous.line, parser->previous.column);
    }

    return parse_primary(parser);
}

Expr *parse_primary(Parser *parser)
{
    if (parser_match(parser, TOKEN_NUMBER))
    {
        if (strchr(parser->previous.lexeme, '.') != NULL)
        {
            return expr_literal_float(parser->previous.literal.float_value,
                                      parser->previous.line, parser->previous.column);
        }
        else
        {
            return expr_literal_number(parser->previous.literal.number_value,
                                       parser->previous.line, parser->previous.column);
        }
    }
    if (parser_match(parser, TOKEN_TRUE))
    {
        return expr_literal_bool(true, parser->previous.line, parser->previous.column);
    }
    if (parser_match(parser, TOKEN_FALSE))
    {
        return expr_literal_bool(false, parser->previous.line, parser->previous.column);
    }
    if (parser_match(parser, TOKEN_NULL))
    {
        return expr_literal_null(parser->previous.line, parser->previous.column);
    }
    if (parser_match(parser, TOKEN_STRING_LITERAL))
    {
        return expr_literal_string(parser->previous.literal.string_value, parser->previous.line, parser->previous.column);
    }
    if (parser_match(parser, TOKEN_IDENTIFIER))
    {
        if (debug_enabled)
        {
            printf("[DEBUG] parse_primary: Found identifier: %s\n", parser->previous.lexeme);
            fflush(stdout);
        }
        return parse_call(parser);
    }
    if (parser_match(parser, TOKEN_LPAREN))
    {
        Expr *expr = parse_expression(parser);
        parser_consume(parser, TOKEN_RPAREN, "Expect ')' after expression.");
        return expr_group(expr, expr->line, expr->column);
    }
    parser_error(parser, "Expect expression.");
    return NULL;
}

Expr *parse_call(Parser *parser)
{
    if (debug_enabled)
    {
        printf("[DEBUG] parse_call: Parsing call for identifier: %s\n", parser->previous.lexeme);
        fflush(stdout);
    }
    Expr *expr = expr_variable(parser->previous.lexeme,
                               parser->previous.line, parser->previous.column);

    while (true)
    {
        if (parser_match(parser, TOKEN_LPAREN))
        {
            expr = finish_call(parser, expr);
        }
        else if (parser_match(parser, TOKEN_LBRACKET))
        {
            expr = finish_array_index(parser, expr);
        }
        else
        {
            break;
        }
    }

    return expr;
}

Expr *finish_call(Parser *parser, Expr *callee)
{
    Expr *call = expr_call(callee->data.variable.name, callee->line, callee->column);
    expr_destroy(callee);

    if (!parser_check(parser, TOKEN_RPAREN))
    {
        do
        {
            if (call->data.call.args.size >= 255)
            {
                parser_error(parser, "Cannot have more than 255 arguments.");
            }
            expr_add_call_arg(call, parse_expression(parser));
        } while (parser_match(parser, TOKEN_COMMA));
    }

    parser_consume(parser, TOKEN_RPAREN, "Expect ')' after arguments.");
    return call;
}

Expr *finish_array_index(Parser *parser, Expr *array)
{
    Expr *index = parse_expression(parser);
    parser_consume(parser, TOKEN_RBRACKET, "Expect ']' after array index.");

    Expr *array_index = expr_array_index(array, index, array->line, array->column);
    return array_index;
}

Expr *finish_string_index(Parser *parser, Expr *string)
{
    Expr *index = parse_expression(parser);
    parser_consume(parser, TOKEN_RBRACKET, "Expect ']' after string index.");

    Expr *string_index = expr_string_index(string, index, string->line, string->column);
    return string_index;
}

Stmt *parse_statement(Parser *parser)
{
    if (debug_enabled)
    {
        printf("[DEBUG] Entered parse_statement, token: %s\n", token_type_to_string(parser->current.type));
        fflush(stdout);
    }
    Stmt *result = NULL;

    if (parser_match(parser, TOKEN_LET))
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Parsing var declaration\n");
            fflush(stdout);
        }
        result = parse_var_declaration(parser);
    }
    else if (parser_match(parser, TOKEN_IF))
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Parsing if statement\n");
            fflush(stdout);
        }
        result = parse_if_statement(parser);
    }
    else if (parser_match(parser, TOKEN_WHILE))
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Parsing while statement\n");
            fflush(stdout);
        }
        result = parse_while_statement(parser);
    }
    else if (parser_match(parser, TOKEN_BREAK))
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Parsing break statement\n");
            fflush(stdout);
        }
        result = parse_break_statement(parser);
    }
    else if (parser_match(parser, TOKEN_CONTINUE))
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Parsing continue statement\n");
            fflush(stdout);
        }
        result = parse_continue_statement(parser);
    }
    else if (parser_match(parser, TOKEN_RETURN))
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Parsing return statement\n");
            fflush(stdout);
        }
        result = parse_return_statement(parser);
    }
    else if (parser_match(parser, TOKEN_PRINT))
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Parsing print statement\n");
            fflush(stdout);
        }
        result = parse_print_statement(parser);
    }
    else if (parser_match(parser, TOKEN_INCLUDE))
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Parsing include directive\n");
            fflush(stdout);
        }
        result = parse_include_directive(parser);
    }
    else if (parser_match(parser, TOKEN_ASM))
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Parsing inline assembly\n");
            fflush(stdout);
        }
        result = parse_inline_asm(parser);
    }
    else if (parser_match(parser, TOKEN_LBRACE))
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Parsing block\n");
            fflush(stdout);
        }
        result = parse_block(parser);
    }
    else if (parser_check(parser, TOKEN_IDENTIFIER))
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Parsing assignment or expression statement\n");
            fflush(stdout);
        }
        result = parse_assignment(parser);
    }
    else
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Parsing expression statement\n");
            fflush(stdout);
        }
        result = parse_expression_statement(parser);
    }

    if (result)
    {
        parser_reset_error_count(parser);
    }

    if (debug_enabled)
    {
        printf("[DEBUG] Exiting parse_statement\n");
        fflush(stdout);
    }
    return result;
}

Stmt *parse_var_declaration(Parser *parser)
{
    parser_consume(parser, TOKEN_IDENTIFIER, "Expect variable name.");
    char *name = string_copy(parser->previous.lexeme);

    parser_consume(parser, TOKEN_COLON, "Expect ':' after variable name.");

    DataType type = TYPE_INT;
    if (parser_match(parser, TOKEN_INT))
    {
        type = TYPE_INT;
    }
    else if (parser_match(parser, TOKEN_INT8))
    {
        type = TYPE_INT; // Map to INT for now
    }
    else if (parser_match(parser, TOKEN_INT16))
    {
        type = TYPE_INT; // Map to INT for now
    }
    else if (parser_match(parser, TOKEN_INT32))
    {
        type = TYPE_INT; // Map to INT for now
    }
    else if (parser_match(parser, TOKEN_INT64))
    {
        type = TYPE_INT; // Map to INT for now
    }
    else if (parser_match(parser, TOKEN_BOOL))
    {
        type = TYPE_BOOL;
    }
    else if (parser_match(parser, TOKEN_FLOAT))
    {
        type = TYPE_FLOAT;
    }
    else if (parser_match(parser, TOKEN_DOUBLE))
    {
        type = TYPE_DOUBLE;
    }
    else if (parser_match(parser, TOKEN_STRING_TYPE))
    {
        type = TYPE_STRING;
    }
    else
    {
        parser_error(parser, "Expect type annotation.");
    }

    if (parser_match(parser, TOKEN_LBRACKET))
    {
        if (parser_match(parser, TOKEN_NUMBER))
        {
            int size = (int)parser->previous.literal.number_value;
            parser_consume(parser, TOKEN_RBRACKET, "Expect ']' after array size.");

            Expr *initializer = NULL;
            if (parser_match(parser, TOKEN_ASSIGN))
            {
                initializer = parse_expression(parser);
            }

            if (!parser_match(parser, TOKEN_SEMICOLON))
            {
                parser_error(parser, "Expect ';' after array declaration.");
                parser_synchronize(parser);
            }
            return stmt_array_decl(name, type, size, initializer, parser->previous.line, parser->previous.column);
        }
        else
        {
            parser_error(parser, "Expect array size.");
        }
    }

    Expr *initializer = NULL;
    if (parser_match(parser, TOKEN_ASSIGN))
    {
        initializer = parse_expression(parser);
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Expect ';' after variable declaration.");
        parser_synchronize(parser);
    }

    return stmt_var_decl(name, type, initializer, parser->previous.line, parser->previous.column);
}

Stmt *parse_assignment(Parser *parser)
{
    Expr *expr = parse_expression(parser);

    if (parser_match(parser, TOKEN_ASSIGN))
    {
        Expr *value = parse_expression(parser);

        if (expr->type == EXPR_VARIABLE)
        {
            char *name = string_copy(expr->data.variable.name);
            expr_destroy(expr);
            if (!parser_match(parser, TOKEN_SEMICOLON))
            {
                parser_error(parser, "Expect ';' after assignment.");
                parser_synchronize(parser);
            }
            return stmt_assignment(name, value, parser->previous.line, parser->previous.column);
        }
        else if (expr->type == EXPR_ARRAY_INDEX)
        {
            Expr *array = expr->data.array_index.array;
            Expr *index = expr->data.array_index.index;
            if (!parser_match(parser, TOKEN_SEMICOLON))
            {
                parser_error(parser, "Expect ';' after assignment.");
                parser_synchronize(parser);
            }
            return stmt_array_assignment(array, index, value, parser->previous.line, parser->previous.column);
        }

        parser_error(parser, "Invalid assignment target.");
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Expect ';' after expression.");
        parser_synchronize(parser);
    }
    return stmt_expr(expr, expr->line, expr->column);
}

Stmt *parse_if_statement(Parser *parser)
{
    if (debug_enabled)
    {
        printf("[DEBUG] Entered parse_if_statement\n");
        fflush(stdout);
    }
    parser_consume(parser, TOKEN_LPAREN, "Expect '(' after 'if'.");
    if (debug_enabled)
    {
        printf("[DEBUG] Got LPAREN after if\n");
        fflush(stdout);
    }
    Expr *condition = parse_expression(parser);
    if (debug_enabled)
    {
        printf("[DEBUG] Parsed condition\n");
        fflush(stdout);
    }
    parser_consume(parser, TOKEN_RPAREN, "Expect ')' after if condition.");
    if (debug_enabled)
    {
        printf("[DEBUG] Got RPAREN after condition\n");
        fflush(stdout);
    }

    Stmt *then_branch;
    if (parser_match(parser, TOKEN_LBRACE))
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Parsing then branch as block\n");
            fflush(stdout);
        }
        then_branch = parse_block(parser);
    }
    else
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Parsing then branch as single statement\n");
            fflush(stdout);
        }
        then_branch = parse_statement(parser);
    }
    if (debug_enabled)
    {
        printf("[DEBUG] Parsed then branch\n");
        fflush(stdout);
    }

    Stmt *else_branch = NULL;
    if (parser_match(parser, TOKEN_ELSE))
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Found else branch\n");
            fflush(stdout);
        }
        if (parser_match(parser, TOKEN_LBRACE))
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Parsing else branch as block\n");
                fflush(stdout);
            }
            else_branch = parse_block(parser);
        }
        else
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Parsing else branch as single statement\n");
                fflush(stdout);
            }
            else_branch = parse_statement(parser);
        }
    }

    if (debug_enabled)
    {
        printf("[DEBUG] Exiting parse_if_statement\n");
        fflush(stdout);
    }
    return stmt_if(condition, then_branch, else_branch,
                   parser->previous.line, parser->previous.column);
}

Stmt *parse_while_statement(Parser *parser)
{
    parser_consume(parser, TOKEN_LPAREN, "Expect '(' after 'while'.");
    Expr *condition = parse_expression(parser);
    parser_consume(parser, TOKEN_RPAREN, "Expect ')' after condition.");

    Stmt *body = parse_statement(parser);

    return stmt_while(condition, body, parser->previous.line, parser->previous.column);
}

Stmt *parse_break_statement(Parser *parser)
{
    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Expect ';' after 'break'.");
        parser_synchronize(parser);
    }
    return stmt_break(parser->previous.line, parser->previous.column);
}

Stmt *parse_continue_statement(Parser *parser)
{
    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Expect ';' after 'continue'.");
        parser_synchronize(parser);
    }
    return stmt_continue(parser->previous.line, parser->previous.column);
}

Stmt *parse_return_statement(Parser *parser)
{
    Expr *value = NULL;
    if (!parser_check(parser, TOKEN_SEMICOLON))
    {
        value = parse_expression(parser);
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Expect ';' after return value.");
        parser_synchronize(parser);
    }
    return stmt_return(value, parser->previous.line, parser->previous.column);
}

Stmt *parse_print_statement(Parser *parser)
{
    parser_consume(parser, TOKEN_LPAREN, "Expect '(' after 'print'.");

    Stmt *print_stmt = stmt_print_stmt(parser->previous.line, parser->previous.column);

    Expr *first_arg = parse_expression(parser);
    stmt_add_print_arg(print_stmt, first_arg);

    while (parser_match(parser, TOKEN_COMMA))
    {
        Expr *arg = parse_expression(parser);
        stmt_add_print_arg(print_stmt, arg);
    }

    parser_consume(parser, TOKEN_RPAREN, "Expect ')' after print arguments.");
    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Expect ';' after print statement.");
        parser_synchronize(parser);
    }

    return print_stmt;
}

Stmt *parse_expression_statement(Parser *parser)
{
    Expr *expr = parse_expression(parser);
    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Expect ';' after expression.");
        parser_synchronize(parser);
    }
    return stmt_expr(expr, expr->line, expr->column);
}

Stmt *parse_block(Parser *parser)
{
    if (debug_enabled)
    {
        printf("[DEBUG] Entered parse_block\n");
        fflush(stdout);
    }
    Stmt *block = stmt_block(parser->previous.line, parser->previous.column);

    while (!parser_check(parser, TOKEN_RBRACE) && !parser_check(parser, TOKEN_EOF))
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Parsing statement, current token: %s\n", token_type_to_string(parser->current.type));
            fflush(stdout);
        }
        Stmt *stmt = parse_statement(parser);
        if (stmt)
        {
            stmt_add_block_stmt(block, stmt);
            if (debug_enabled)
            {
                printf("[DEBUG] Added statement to block\n");
                fflush(stdout);
            }
        }
        else
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Failed to parse statement\n");
                fflush(stdout);
            }
            break;
        }
    }

    if (debug_enabled)
    {
        printf("[DEBUG] Exiting block, consuming RBRACE\n");
        fflush(stdout);
    }
    if (!parser_match(parser, TOKEN_RBRACE))
    {
        parser_error(parser, "Expect '}' after block.");
        parser_synchronize(parser);
    }
    if (debug_enabled)
    {
        printf("[DEBUG] Exiting parse_block\n");
        fflush(stdout);
    }
    return block;
}

Function *parse_function_declaration(Parser *parser)
{
    if (debug_enabled)
    {
        printf("[DEBUG] Entered parse_function_declaration\n");
        fflush(stdout);
    }
    parser_consume(parser, TOKEN_IDENTIFIER, "Expect function name.");
    char *name = string_copy(parser->previous.lexeme);
    if (debug_enabled)
    {
        printf("[DEBUG] Function declaration name: %s\n", name);
        fflush(stdout);
    }

    parser_consume(parser, TOKEN_LPAREN, "Expect '(' after function name.");
    if (debug_enabled)
    {
        printf("[DEBUG] Got LPAREN\n");
        fflush(stdout);
    }

    Function *function = function_create(name, TYPE_INT);
    if (!parser_check(parser, TOKEN_RPAREN))
    {
        do
        {
            if (function->params.size >= 255)
            {
                parser_error(parser, "Cannot have more than 255 parameters.");
            }
            function_add_param(function, parse_parameter(parser));
        } while (parser_match(parser, TOKEN_COMMA));
    }

    parser_consume(parser, TOKEN_RPAREN, "Expect ')' after parameters.");
    if (debug_enabled)
    {
        printf("[DEBUG] Got RPAREN\n");
        fflush(stdout);
    }
    parser_consume(parser, TOKEN_ARROW, "Expect '->' after function parameters.");
    if (debug_enabled)
    {
        printf("[DEBUG] Got ARROW\n");
        fflush(stdout);
    }

    if (parser_match(parser, TOKEN_INT))
    {
        function->return_type = TYPE_INT;
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: INT\n");
            fflush(stdout);
        }
    }
    else if (parser_match(parser, TOKEN_INT8))
    {
        function->return_type = TYPE_INT; // Map to INT for now
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: INT8\n");
            fflush(stdout);
        }
    }
    else if (parser_match(parser, TOKEN_INT16))
    {
        function->return_type = TYPE_INT; // Map to INT for now
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: INT16\n");
            fflush(stdout);
        }
    }
    else if (parser_match(parser, TOKEN_INT32))
    {
        function->return_type = TYPE_INT; // Map to INT for now
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: INT32\n");
            fflush(stdout);
        }
    }
    else if (parser_match(parser, TOKEN_INT64))
    {
        function->return_type = TYPE_INT; // Map to INT for now (can be extended later)
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: INT64\n");
            fflush(stdout);
        }
    }
    else if (parser_match(parser, TOKEN_BOOL))
    {
        function->return_type = TYPE_BOOL;
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: BOOL\n");
            fflush(stdout);
        }
    }
    else if (parser_match(parser, TOKEN_FLOAT))
    {
        function->return_type = TYPE_FLOAT;
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: FLOAT\n");
            fflush(stdout);
        }
    }
    else if (parser_match(parser, TOKEN_DOUBLE))
    {
        function->return_type = TYPE_DOUBLE;
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: DOUBLE\n");
            fflush(stdout);
        }
    }
    else if (parser_match(parser, TOKEN_STRING_TYPE))
    {
        function->return_type = TYPE_STRING;
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: STRING\n");
            fflush(stdout);
        }
    }
    else
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Expected return type, got: %s\n", token_type_to_string(parser->current.type));
            fflush(stdout);
        }
        parser_error(parser, "Expect return type.");
        function_destroy(function);
        return NULL;
    }

    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Expect ';' after function declaration.");
        function_destroy(function);
        return NULL;
    }

    if (debug_enabled)
    {
        printf("[DEBUG] Finished parsing function declaration\n");
        fflush(stdout);
    }

    return function;
}

Function *parse_function(Parser *parser)
{
    if (debug_enabled)
    {
        printf("[DEBUG] Entered parse_function\n");
        fflush(stdout);
    }
    parser_consume(parser, TOKEN_IDENTIFIER, "Expect function name.");
    char *name = string_copy(parser->previous.lexeme);
    if (debug_enabled)
    {
        printf("[DEBUG] Function name: %s\n", name);
        fflush(stdout);
    }

    parser_consume(parser, TOKEN_LPAREN, "Expect '(' after function name.");
    if (debug_enabled)
    {
        printf("[DEBUG] Got LPAREN\n");
        fflush(stdout);
    }

    Function *function = function_create(name, TYPE_INT);
    if (!parser_check(parser, TOKEN_RPAREN))
    {
        do
        {
            if (function->params.size >= 255)
            {
                parser_error(parser, "Cannot have more than 255 parameters.");
            }
            function_add_param(function, parse_parameter(parser));
        } while (parser_match(parser, TOKEN_COMMA));
    }

    parser_consume(parser, TOKEN_RPAREN, "Expect ')' after parameters.");
    if (debug_enabled)
    {
        printf("[DEBUG] Got RPAREN\n");
        fflush(stdout);
    }
    parser_consume(parser, TOKEN_ARROW, "Expect '->' after function parameters.");
    if (debug_enabled)
    {
        printf("[DEBUG] Got ARROW\n");
        fflush(stdout);
    }

    if (parser_match(parser, TOKEN_INT))
    {
        function->return_type = TYPE_INT;
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: INT\n");
            fflush(stdout);
        }
    }
    else if (parser_match(parser, TOKEN_INT8))
    {
        function->return_type = TYPE_INT; // Map to INT for now
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: INT8\n");
            fflush(stdout);
        }
    }
    else if (parser_match(parser, TOKEN_INT16))
    {
        function->return_type = TYPE_INT; // Map to INT for now
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: INT16\n");
            fflush(stdout);
        }
    }
    else if (parser_match(parser, TOKEN_INT32))
    {
        function->return_type = TYPE_INT; // Map to INT for now
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: INT32\n");
            fflush(stdout);
        }
    }
    else if (parser_match(parser, TOKEN_INT64))
    {
        function->return_type = TYPE_INT; // Map to INT for now (can be extended later)
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: INT64\n");
            fflush(stdout);
        }
    }
    else if (parser_match(parser, TOKEN_BOOL))
    {
        function->return_type = TYPE_BOOL;
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: BOOL\n");
            fflush(stdout);
        }
    }
    else if (parser_match(parser, TOKEN_FLOAT))
    {
        function->return_type = TYPE_FLOAT;
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: FLOAT\n");
            fflush(stdout);
        }
    }
    else if (parser_match(parser, TOKEN_DOUBLE))
    {
        function->return_type = TYPE_DOUBLE;
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: DOUBLE\n");
            fflush(stdout);
        }
    }
    else if (parser_match(parser, TOKEN_STRING_TYPE))
    {
        function->return_type = TYPE_STRING;
        if (debug_enabled)
        {
            printf("[DEBUG] Return type: STRING\n");
            fflush(stdout);
        }
    }
    else
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Expected return type, got: %s\n", token_type_to_string(parser->current.type));
            fflush(stdout);
        }
        parser_error(parser, "Expect return type.");
        function_destroy(function);
        return NULL;
    }

    parser_consume(parser, TOKEN_LBRACE, "Expect '{' before function body.");
    if (debug_enabled)
    {
        printf("[DEBUG] Got LBRACE, parsing body\n");
        fflush(stdout);
    }
    function->body = parse_block(parser);
    if (debug_enabled)
    {
        printf("[DEBUG] Finished parsing function body\n");
        fflush(stdout);
    }

    return function;
}

Parameter *parse_parameter(Parser *parser)
{
    parser_consume(parser, TOKEN_IDENTIFIER, "Expect parameter name.");
    char *name = string_copy(parser->previous.lexeme);

    parser_consume(parser, TOKEN_COLON, "Expect ':' after parameter name.");

    DataType type = TYPE_INT;
    if (parser_match(parser, TOKEN_INT))
    {
        type = TYPE_INT;
    }
    else if (parser_match(parser, TOKEN_INT8))
    {
        type = TYPE_INT; // Map to INT for now
    }
    else if (parser_match(parser, TOKEN_INT16))
    {
        type = TYPE_INT; // Map to INT for now
    }
    else if (parser_match(parser, TOKEN_INT32))
    {
        type = TYPE_INT; // Map to INT for now
    }
    else if (parser_match(parser, TOKEN_INT64))
    {
        type = TYPE_INT; // Map to INT for now
    }
    else if (parser_match(parser, TOKEN_BOOL))
    {
        type = TYPE_BOOL;
    }
    else if (parser_match(parser, TOKEN_FLOAT))
    {
        type = TYPE_FLOAT;
    }
    else if (parser_match(parser, TOKEN_DOUBLE))
    {
        type = TYPE_DOUBLE;
    }
    else if (parser_match(parser, TOKEN_STRING_TYPE))
    {
        type = TYPE_STRING;
    }
    else
    {
        parser_error(parser, "Expect parameter type.");
    }

    return parameter_create(name, type);
}

Program *parser_parse(Parser *parser)
{
    if (debug_enabled)
    {
        printf("[DEBUG] Entered parser_parse\n");
        fflush(stdout);
    }
    Program *program = program_create();

    while (!parser_check(parser, TOKEN_EOF))
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Current token: %s\n", token_type_to_string(parser->current.type));
            fflush(stdout);
        }
        if (parser_match(parser, TOKEN_FUNC))
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Parsing function\n");
                fflush(stdout);
            }
            Function *func = parse_function(parser);
            if (func)
            {
                program_add_function(program, func);
                if (debug_enabled)
                {
                    printf("[DEBUG] Added function to program\n");
                    fflush(stdout);
                }
            }
            else
            {
                if (debug_enabled)
                {
                    printf("[DEBUG] Failed to parse function\n");
                    fflush(stdout);
                }
                break;
            }
        }
        else if (parser_match(parser, TOKEN_INCLUDE))
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Parsing include directive\n");
                fflush(stdout);
            }
            Stmt *include_stmt = parse_include_directive(parser);
            if (include_stmt)
            {
                if (debug_enabled)
                {
                    printf("[DEBUG] Parsed include directive: %s\n", include_stmt->data.include.path);
                    fflush(stdout);
                }
                program_add_include(program, include_stmt);
            }
            else
            {
                if (debug_enabled)
                {
                    printf("[DEBUG] Failed to parse include directive\n");
                    fflush(stdout);
                }
                break;
            }
        }
        else if (parser_match(parser, TOKEN_EXTERN))
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Parsing extern declaration (minimal)\n");
                fflush(stdout);
            }
            if (!parser_check(parser, TOKEN_STRING_LITERAL))
            {
                parser_error(parser, "Expect calling convention string after 'extern'.");
                break;
            }
            char *calling_convention = string_copy(parser->current.literal.string_value);
            parser_advance(parser);
            
            char *library_name = string_copy("kernel32.dll"); 
            if (parser_match(parser, TOKEN_FROM))
            {
                if (!parser_check(parser, TOKEN_STRING_LITERAL))
                {
                    parser_error(parser, "Expect library name after 'from'.");
                    safe_free(calling_convention);
                    safe_free(library_name);
                    break;
                }
                safe_free(library_name);
                library_name = string_copy(parser->current.literal.string_value);
                parser_advance(parser);
            }
            
            parser_consume(parser, TOKEN_LBRACE, "Expect '{' after calling convention.");
            
            while (!parser_check(parser, TOKEN_RBRACE) && !parser_check(parser, TOKEN_EOF))
            {
                if (!parser_match(parser, TOKEN_FUNC))
                {
                    parser_error(parser, "Expect 'func' inside extern block.");
                    break;
                }
                
                if (!parser_check(parser, TOKEN_IDENTIFIER))
                {
                    parser_error(parser, "Expect function name.");
                    break;
                }
                char *func_name = string_copy(parser->current.lexeme);
                parser_advance(parser);
                
                parser_consume(parser, TOKEN_LPAREN, "Expect '(' after function name.");
                
                FFIFunction *ffi_func = ffi_function_create(func_name, library_name, calling_convention, (int)TYPE_INT);
                
                if (!parser_check(parser, TOKEN_RPAREN))
                {
                    do
                    {
                        if (!parser_check(parser, TOKEN_IDENTIFIER))
                        {
                            parser_error(parser, "Expect parameter name.");
                            break;
                        }
                        char *param_name = string_copy(parser->current.lexeme);
                        parser_advance(parser);
                        
                        parser_consume(parser, TOKEN_COLON, "Expect ':' after parameter name.");
                        DataType param_type = token_to_data_type(parser->current.type);
                        if (param_type == TYPE_NULL)
                        {
                            parser_error(parser, "Expect valid parameter type.");
                            safe_free(param_name);
                            break;
                        }
                        parser_advance(parser);
                        
                        if (debug_enabled)
                        {
                            printf("[DEBUG] Creating parameter: %s with type %d\n", param_name, param_type);
                            fflush(stdout);
                        }
                        Parameter *param = parameter_create(param_name, param_type);
                        ffi_function_add_param(ffi_func, param);
                        safe_free(param_name);
                        if (debug_enabled)
                        {
                            printf("[DEBUG] Added parameter to FFI function\n");
                            fflush(stdout);
                        }
                    } while (parser_match(parser, TOKEN_COMMA));
                }
                
                parser_consume(parser, TOKEN_RPAREN, "Expect ')' after function parameters.");
                parser_consume(parser, TOKEN_ARROW, "Expect '->' after function parameters.");
                
                DataType return_type = token_to_data_type(parser->current.type);
                if (return_type == TYPE_NULL)
                {
                    parser_error(parser, "Expect valid return type.");
                    safe_free(func_name);
                    ffi_function_destroy(ffi_func);
                    break;
                }
                parser_advance(parser);
                
                ffi_func->return_type = (int)return_type;
                
                parser_consume(parser, TOKEN_SEMICOLON, "Expect ';' after function declaration.");
                if (ffi_func)
                {
                    program_add_ffi_function(program, ffi_func);
                    if (debug_enabled)
                    {
                        printf("[DEBUG] Created FFI function: %s\n", func_name);
                        fflush(stdout);
                    }
                }
                
                safe_free(func_name);
            }
            
            parser_consume(parser, TOKEN_RBRACE, "Expect '}' after extern block.");
            
            safe_free(calling_convention);
            safe_free(library_name);
            
            if (debug_enabled)
            {
                printf("[DEBUG] Parsed extern block successfully\n");
                fflush(stdout);
            }
        }
        else
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Expected function declaration or include directive, got: %s\n", token_type_to_string(parser->current.type));
                fflush(stdout);
            }
            parser_error(parser, "Expect function declaration or include directive.");
            break;
        }
    }

    if (debug_enabled)
    {
        printf("[DEBUG] Exiting parser_parse, had_error: %d\n", parser->had_error);
        fflush(stdout);
    }
    return program;
}

Stmt *parse_include_directive(Parser *parser)
{
    if (!parser_check(parser, TOKEN_STRING_LITERAL))
    {
        parser_error(parser, "Expect string literal after #include");
        return NULL;
    }

    char *path = string_copy(parser->current.literal.string_value);
    IncludeType type = INCLUDE_LOCAL;

    if (path[0] == '<' && path[strlen(path) - 1] == '>')
    {
        type = INCLUDE_SYSTEM;
        char *temp = string_copy(path + 1);
        safe_free(path);
        path = temp;
        path[strlen(path) - 1] = '\0';
    }
    else if (path[0] == '"' && path[strlen(path) - 1] == '"')
    {
        char *temp = string_copy(path + 1);
        safe_free(path);
        path = temp;
        path[strlen(path) - 1] = '\0';
    }

    parser_advance(parser);

    return stmt_include(path, type, parser->previous.line, parser->previous.column);
}

Stmt *parse_inline_asm(Parser *parser)
{
    int line = parser->previous.line;
    int column = parser->previous.column;
    
    bool is_volatile = parser_match(parser, TOKEN_VOLATILE);
    
    parser_consume(parser, TOKEN_LBRACE, "Expect '{' after 'asm'");
    
    if (!parser_check(parser, TOKEN_STRING_LITERAL))
    {
        parser_error(parser, "Expect assembly code string");
        return NULL;
    }
    
    DynamicArray string_parts;
    array_init(&string_parts, 4);
    
    while (parser_check(parser, TOKEN_STRING_LITERAL))
    {
        char *str_part = string_copy(parser->current.literal.string_value);
        if (str_part[0] == '"' && str_part[strlen(str_part) - 1] == '"')
        {
            char *temp = string_copy(str_part + 1);
            safe_free(str_part);
            str_part = temp;
            str_part[strlen(str_part) - 1] = '\0';
        }
        array_push(&string_parts, str_part);
        parser_advance(parser);
    }
    
    size_t total_len = 0;
    for (size_t i = 0; i < string_parts.size; i++)
    {
        char *part = (char*)array_get(&string_parts, i);
        total_len += strlen(part);
    }
    
    char *asm_code = safe_malloc(total_len + 1);
    asm_code[0] = '\0';
    for (size_t i = 0; i < string_parts.size; i++)
    {
        char *part = (char*)array_get(&string_parts, i);
        strcat(asm_code, part);
        safe_free(part);
    }
    array_free(&string_parts);
    
    Stmt *stmt = stmt_inline_asm(asm_code, is_volatile, line, column);
    safe_free(asm_code);
    
    if (parser_match(parser, TOKEN_COLON))
    {
        while (true)
        {
            if (!parser_check(parser, TOKEN_STRING_LITERAL))
                break;
            
            char *constraint = string_copy(parser->current.literal.string_value);
            if (constraint[0] == '"' && constraint[strlen(constraint) - 1] == '"')
            {
                char *temp = string_copy(constraint + 1);
                safe_free(constraint);
                constraint = temp;
                constraint[strlen(constraint) - 1] = '\0';
            }
            parser_advance(parser);
            
            parser_consume(parser, TOKEN_LPAREN, "Expect '(' after constraint");
            if (!parser_check(parser, TOKEN_IDENTIFIER))
            {
                parser_error(parser, "Expect variable name");
                safe_free(constraint);
                break;
            }
            char *variable = string_copy(parser->current.lexeme);
            parser_advance(parser);
            parser_consume(parser, TOKEN_RPAREN, "Expect ')' after variable");
            
            stmt_add_inline_asm_output(stmt, constraint, variable);
            safe_free(constraint);
            safe_free(variable);
            
            if (!parser_match(parser, TOKEN_COMMA))
                break;
        }
        
        if (debug_enabled)
        {
            printf("[DEBUG] Before checking for colon after outputs, current token: %s\n",
                   token_type_to_string(parser->current.type));
            fflush(stdout);
        }
        if (parser_match(parser, TOKEN_COLON))
        {
            if (debug_enabled)
            {
                printf("[DEBUG] After matching colon, current token: %s (type %d), lexeme: %s\n",
                       token_type_to_string(parser->current.type),
                       parser->current.type,
                       parser->current.lexeme ? parser->current.lexeme : "(null)");
                fflush(stdout);
            }
            bool had_empty_inputs = false;
            if (parser_check(parser, TOKEN_COLON))
            {
                parser_advance(parser);
                had_empty_inputs = true;
            }
            else
            {
                if (debug_enabled)
                {
                    printf("[DEBUG] Not a colon, checking if string literal. Current type: %d (%s), TOKEN_STRING_LITERAL=%d\n",
                           parser->current.type, token_type_to_string(parser->current.type), TOKEN_STRING_LITERAL);
                    fflush(stdout);
                }
            }
            if (debug_enabled)
            {
                printf("[DEBUG] Checking condition: !had_empty_inputs=%d, parser_check(TOKEN_STRING_LITERAL)=%d, current type=%d\n",
                       !had_empty_inputs, parser_check(parser, TOKEN_STRING_LITERAL), parser->current.type);
                fflush(stdout);
            }
            if (!had_empty_inputs && parser_check(parser, TOKEN_STRING_LITERAL))
            {
                if (debug_enabled)
                {
                    printf("[DEBUG] Found string literal after colon, checking if input or clobber\n");
                    fflush(stdout);
                }

                bool is_input = false;
                Token peek = lexer_peek_token(parser->lexer);
                is_input = (peek.type == TOKEN_LPAREN);
                token_destroy(&peek);
                
                if (debug_enabled)
                {
                    printf("[DEBUG] After peek, is_input=%d, current token type: %d (%s)\n", 
                           is_input, parser->current.type, token_type_to_string(parser->current.type));
                    fflush(stdout);
                }
                
                if (is_input)
                {
                    while (true)
                    {
                        if (!parser_check(parser, TOKEN_STRING_LITERAL))
                            break;
                        
                        char *constraint = string_copy(parser->current.literal.string_value);
                        if (constraint[0] == '"' && constraint[strlen(constraint) - 1] == '"')
                        {
                            char *temp = string_copy(constraint + 1);
                            safe_free(constraint);
                            constraint = temp;
                            constraint[strlen(constraint) - 1] = '\0';
                        }
                        parser_advance(parser);
                        
                        parser_consume(parser, TOKEN_LPAREN, "Expect '(' after constraint");
                        char *variable = NULL;
                        if (parser_check(parser, TOKEN_IDENTIFIER))
                        {
                            variable = string_copy(parser->current.lexeme);
                            parser_advance(parser);
                        }
                        else if (parser_check(parser, TOKEN_NUMBER))
                        {
                            char num_str[64];
                            snprintf(num_str, sizeof(num_str), "%lld", parser->current.literal.number_value);
                            variable = string_copy(num_str);
                            parser_advance(parser);
                        }
                        else
                        {
                            parser_error(parser, "Expect variable name or number");
                            safe_free(constraint);
                            break;
                        }
                        parser_consume(parser, TOKEN_RPAREN, "Expect ')' after input operand");
                        
                        stmt_add_inline_asm_input(stmt, constraint, variable);
                        safe_free(constraint);
                        safe_free(variable);
                        
                        if (!parser_match(parser, TOKEN_COMMA))
                            break;
                    }
                    
                    if (parser_match(parser, TOKEN_COLON))
                    {
                        while (true)
                        {
                            if (!parser_check(parser, TOKEN_STRING_LITERAL))
                                break;
                            
                            char *clobber = string_copy(parser->current.literal.string_value);
                            if (clobber[0] == '"' && clobber[strlen(clobber) - 1] == '"')
                            {
                                char *temp = string_copy(clobber + 1);
                                safe_free(clobber);
                                clobber = temp;
                                clobber[strlen(clobber) - 1] = '\0';
                            }
                            parser_advance(parser);
                            
                            stmt_add_inline_asm_clobber(stmt, clobber);
                            safe_free(clobber);
                            
                            if (!parser_match(parser, TOKEN_COMMA))
                                break;
                        }
                    }
                }
                else
                {
                    if (debug_enabled)
                    {
                        printf("[DEBUG] Entering clobbers section (no inputs), current token: %s, lexeme: %s\n", 
                               token_type_to_string(parser->current.type),
                               parser->current.lexeme ? parser->current.lexeme : "(null)");
                        fflush(stdout);
                    }
                    while (true)
                    {
                        if (debug_enabled)
                        {
                            printf("[DEBUG] Checking for string literal in clobbers loop, current: %s\n", 
                                   token_type_to_string(parser->current.type));
                            fflush(stdout);
                        }
                        if (!parser_check(parser, TOKEN_STRING_LITERAL))
                            break;
                        
                        char *clobber = string_copy(parser->current.literal.string_value);
                        if (clobber[0] == '"' && clobber[strlen(clobber) - 1] == '"')
                        {
                            char *temp = string_copy(clobber + 1);
                            safe_free(clobber);
                            clobber = temp;
                            clobber[strlen(clobber) - 1] = '\0';
                        }
                        parser_advance(parser);
                        
                        stmt_add_inline_asm_clobber(stmt, clobber);
                        safe_free(clobber);
                        
                        if (!parser_match(parser, TOKEN_COMMA))
                            break;
                    }
                    if (debug_enabled)
                    {
                        printf("[DEBUG] After parsing clobbers (no inputs), current token: %s (line %d, col %d)\n", 
                               token_type_to_string(parser->current.type),
                               parser->current.line, parser->current.column);
                        fflush(stdout);
                    }
                }
            }
            else
            {
                if (parser->current.type == TOKEN_STRING_LITERAL)
                {
                    if (debug_enabled)
                    {
                        printf("[DEBUG] Found string literal in else branch, treating as clobber\n");
                        fflush(stdout);
                    }
                    while (true)
                    {
                        if (parser->current.type != TOKEN_STRING_LITERAL)
                            break;
                        
                        char *clobber = string_copy(parser->current.literal.string_value);
                        if (clobber[0] == '"' && clobber[strlen(clobber) - 1] == '"')
                        {
                            char *temp = string_copy(clobber + 1);
                            safe_free(clobber);
                            clobber = temp;
                            clobber[strlen(clobber) - 1] = '\0';
                        }
                        parser_advance(parser);
                        
                        stmt_add_inline_asm_clobber(stmt, clobber);
                        safe_free(clobber);
                        
                        if (!parser_match(parser, TOKEN_COMMA))
                            break;
                    }
                }
                else
                {
                    if (debug_enabled)
                    {
                        printf("[DEBUG] After colon, expected string literal but got: %s\n", 
                               token_type_to_string(parser->current.type));
                        fflush(stdout);
                    }
                }
            }
            
            if (debug_enabled)
            {
                printf("[DEBUG] Before checking had_empty_inputs, current token: %s\n", 
                       token_type_to_string(parser->current.type));
                fflush(stdout);
            }
            
            if (had_empty_inputs && parser_check(parser, TOKEN_COLON))
            {
                parser_advance(parser); 
                while (true)
                {
                    if (!parser_check(parser, TOKEN_STRING_LITERAL))
                        break;
                    
                    char *clobber = string_copy(parser->current.literal.string_value);
                    if (clobber[0] == '"' && clobber[strlen(clobber) - 1] == '"')
                    {
                        char *temp = string_copy(clobber + 1);
                        safe_free(clobber);
                        clobber = temp;
                        clobber[strlen(clobber) - 1] = '\0';
                    }
                    parser_advance(parser);
                    
                    stmt_add_inline_asm_clobber(stmt, clobber);
                    safe_free(clobber);
                    
                    if (!parser_match(parser, TOKEN_COMMA))
                        break;
                }
            }
        }
        if (debug_enabled)
        {
            printf("[DEBUG] After exiting outputs block, current token: %s (line %d, col %d)\n", 
                   token_type_to_string(parser->current.type),
                   parser->current.line, parser->current.column);
            fflush(stdout);
        }
    }
    
    if (debug_enabled)
    {
        printf("[DEBUG] Before consuming RBRACE, current token: %s (line %d, col %d)\n", 
               token_type_to_string(parser->current.type), 
               parser->current.line, parser->current.column);
        if (parser->current.lexeme)
        {
            printf("[DEBUG] Current token lexeme: %s\n", parser->current.lexeme);
        }
        fflush(stdout);
    }
    parser_consume(parser, TOKEN_RBRACE, "Expect '}' after inline assembly");
    
    if (!parser_match(parser, TOKEN_SEMICOLON))
    {
        parser_error(parser, "Expect ';' after inline assembly statement");
        parser_synchronize(parser);
    }
    
    return stmt;
}

FFIFunction *parse_extern_declaration(Parser *parser, Program *program)
{
    if (!parser_check(parser, TOKEN_STRING_LITERAL))
    {
        parser_error(parser, "Expect calling convention string after 'extern'.");
        return NULL;
    }
    
    char *calling_convention;
    if (parser->current.literal.string_value) {
        calling_convention = string_copy(parser->current.literal.string_value);
        parser_advance(parser);
    } else {
        parser_error(parser, "Invalid calling convention string after 'extern'.");
        return NULL;
    }
    
    char *library_name = string_copy("kernel32.dll"); 
    if (parser_match(parser, TOKEN_FROM))
    {
        if (!parser_check(parser, TOKEN_STRING_LITERAL))
        {
            parser_error(parser, "Expect library name after 'from'.");
            safe_free(calling_convention);
            safe_free(library_name);
            return NULL;
        }
        safe_free(library_name);
        if (parser->current.literal.string_value) {
            library_name = string_copy(parser->current.literal.string_value);
        } else {
            parser_error(parser, "Invalid string literal after 'from'.");
            safe_free(calling_convention);
            return NULL;
        }
        parser_advance(parser);
    }
    
    parser_consume(parser, TOKEN_LBRACE, "Expect '{' after calling convention.");
    
    FFIFunction *first_func = NULL;
    
    while (!parser_check(parser, TOKEN_RBRACE) && !parser_check(parser, TOKEN_EOF))
    {
        
        if (!parser_match(parser, TOKEN_FUNC))
        {
            parser_error(parser, "Expect 'func' inside extern block.");
            break;
        }
        
        if (!parser_check(parser, TOKEN_IDENTIFIER))
        {
            parser_error(parser, "Expect function name after 'func'.");
            break;
        }
        
        char *func_name = string_copy(parser->current.lexeme);
        parser_advance(parser);
        
        parser_consume(parser, TOKEN_LPAREN, "Expect '(' after function name.");
        
        FFIFunction *ffi_func = ffi_function_create(func_name, library_name, calling_convention, (int)TYPE_INT);
        ffi_func->line = parser->current.line;
        ffi_func->column = parser->current.column;
        
        if (!parser_check(parser, TOKEN_RPAREN))
        {
            do
            {
                if (((DynamicArray*)ffi_func->params)->size >= 255)
                {
                    parser_error(parser, "Cannot have more than 255 parameters.");
                    break;
                }
                
                if (!parser_check(parser, TOKEN_IDENTIFIER))
                {
                    parser_error(parser, "Expect parameter name.");
                    break;
                }
                
                char *param_name = string_copy(parser->current.lexeme);
                parser_advance(parser);
                
                parser_consume(parser, TOKEN_COLON, "Expect ':' after parameter name.");
                
                DataType param_type = token_to_data_type(parser->current.type);
                if (param_type == TYPE_NULL)
                {
                    parser_error(parser, "Expect valid parameter type.");
                    break;
                }
                parser_advance(parser);
                
                Parameter *param = parameter_create(param_name, param_type);
                ffi_function_add_param(ffi_func, param);
                
            } while (parser_match(parser, TOKEN_COMMA));
        }
        
        parser_consume(parser, TOKEN_RPAREN, "Expect ')' after parameters.");
        parser_consume(parser, TOKEN_ARROW, "Expect '->' after function parameters.");
        
        DataType return_type = token_to_data_type(parser->current.type);
        if (return_type == TYPE_NULL)
        {
            parser_error(parser, "Expect valid return type.");
            break;
        }
        parser_advance(parser);
        
        ffi_func->return_type = (int)return_type;
        
        parser_consume(parser, TOKEN_SEMICOLON, "Expect ';' after function declaration.");
        
        program_add_ffi_function(program, ffi_func);
        
        if (!first_func)
        {
            first_func = ffi_func;
        }
        // Note: We don't destroy ffi_func here because it's now owned by the program
    }
    
    parser_consume(parser, TOKEN_RBRACE, "Expect '}' after extern block.");
    
    safe_free(calling_convention);
    safe_free(library_name);
    return first_func;
}