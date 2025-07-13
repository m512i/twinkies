#include "utils.h"
#include "flags.h"
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "frontend/ast.h"
#include "analysis/semantic.h"
#include "backend/ir.h"
#include "backend/codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        print_error("compiler", "cannot open file");
        fprintf(stderr, "  %s\n", filename);
        fflush(stderr);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = safe_malloc(file_size + 1);
    if (!buffer)
    {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0';

    fclose(file);
    return buffer;
}

void print_tokens(const char *source, const char *filename)
{
    if (debug_enabled)
    {
        printf("Tokens for %s:\n", filename);
        printf("================\n");
        fflush(stdout);
    }
    Error error;
    error_init(&error);

    Lexer *lexer = lexer_create(source, &error);
    if (error.type != ERROR_NONE)
    {
        error_print(&error, filename);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }

    Token token;
    do
    {
        token = lexer_next_token(lexer);
        token_print(&token);
        token_destroy(&token);
        fflush(stdout);
    } while (token.type != TOKEN_EOF);

    lexer_destroy(lexer);
    if (debug_enabled)
    {
        printf("[DEBUG] Exiting print_tokens\n");
        fflush(stdout);
    }
}

void print_ast(const char *source, const char *filename)
{
    if (debug_enabled)
    {
        printf("AST for %s:\n", filename);
        printf("============\n");
        fflush(stdout);
    }
    Error error;
    error_init(&error);

    Lexer *lexer = lexer_create(source, &error);
    if (error.type != ERROR_NONE)
    {
        error_print(&error, filename);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }

    ErrorContext *error_context = error_context_create(filename, source);
    Parser *parser = parser_create(lexer, error_context);
    if (error.type != ERROR_NONE)
    {
        error_print(&error, filename);
        error_context_destroy(error_context);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }

    Program *program = parser_parse(parser);
    if (error.type != ERROR_NONE)
    {
        error_print(&error, filename);
        program_destroy(program);
        error_context_destroy(error_context);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }

    program_print(program);
    fflush(stdout);

    program_destroy(program);
    error_context_destroy(error_context);
    parser_destroy(parser);
    lexer_destroy(lexer);
    if (debug_enabled)
    {
        printf("[DEBUG] Exiting print_ast\n");
        fflush(stdout);
    }
}

void print_ir(const char *source, const char *filename)
{
    if (debug_enabled)
    {
        printf("IR for %s:\n", filename);
        printf("===========\n");
        fflush(stdout);
    }
    Error error;
    error_init(&error);

    Lexer *lexer = lexer_create(source, &error);
    if (error.type != ERROR_NONE)
    {
        error_print(&error, filename);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }

    ErrorContext *error_context = error_context_create(filename, source);
    Parser *parser = parser_create(lexer, error_context);
    if (error.type != ERROR_NONE)
    {
        error_print(&error, filename);
        error_context_destroy(error_context);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }

    Program *program = parser_parse(parser);
    if (error.type != ERROR_NONE)
    {
        error_print(&error, filename);
        program_destroy(program);
        error_context_destroy(error_context);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }

    SemanticAnalyzer *analyzer = semantic_create(program, error_context);
    if (error.type != ERROR_NONE)
    {
        error_print(&error, filename);
        semantic_destroy(analyzer);
        program_destroy(program);
        error_context_destroy(error_context);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }

    if (!semantic_analyze(analyzer))
    {
        semantic_destroy(analyzer);
        program_destroy(program);
        error_context_destroy(error_context);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }

    IRProgram *ir_program = ir_generate(program, analyzer);
    if (!ir_program)
    {
        print_error("compiler", "failed to generate IR");
        semantic_destroy(analyzer);
        program_destroy(program);
        error_context_destroy(error_context);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }

    ir_program_print(ir_program);
    fflush(stdout);

    ir_program_destroy(ir_program);
    semantic_destroy(analyzer);
    program_destroy(program);
    error_context_destroy(error_context);
    parser_destroy(parser);
    lexer_destroy(lexer);
    if (debug_enabled)
    {
        printf("[DEBUG] Exiting print_ir\n");
        fflush(stdout);
    }
}

void dump_ast_json(const char *source, const char *filename)
{
    if (debug_enabled)
    {
        printf("{\n");
        printf("  \"ast\": {\n");
        printf("    \"type\": \"program\",\n");
        printf("    \"filename\": \"%s\",\n", filename);
        printf("    \"functions\": [\n");
    }

    Error error;
    error_init(&error);
    Lexer *lexer = lexer_create(source, &error);
    if (error.type != ERROR_NONE)
    {
        error_print(&error, filename);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    ErrorContext *error_context = error_context_create(filename, source);
    Parser *parser = parser_create(lexer, error_context);
    if (error.type != ERROR_NONE)
    {
        error_print(&error, filename);
        error_context_destroy(error_context);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    Program *program = parser_parse(parser);
    if (error.type != ERROR_NONE)
    {
        error_print(&error, filename);
        program_destroy(program);
        error_context_destroy(error_context);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }

    for (size_t i = 0; i < program->functions.size; i++)
    {
        Function *func = (Function *)array_get(&program->functions, i);
        if (debug_enabled)
        {
            printf("      {\n");
            printf("        \"type\": \"function\",\n");
            printf("        \"name\": \"%s\",\n", func->name);
            printf("        \"return_type\": \"%s\",\n", data_type_to_string(func->return_type));
            printf("        \"parameters\": [\n");
        }
        for (size_t j = 0; j < func->params.size; j++)
        {
            Parameter *param = (Parameter *)array_get(&func->params, j);
            if (debug_enabled)
            {
                printf("          {\n");
                printf("            \"name\": \"%s\",\n", param->name);
                printf("            \"type\": \"%s\"\n", data_type_to_string(param->type));
                printf("          }%s\n", j < func->params.size - 1 ? "," : "");
            }
        }
        if (debug_enabled)
        {
            printf("        ],\n");
            printf("        \"body\": ");
            dump_stmt_json(func->body, 8);
            printf("\n      }%s\n", i < program->functions.size - 1 ? "," : "");
        }
    }
    if (debug_enabled)
    {
        printf("    ]\n");
        printf("  }\n");
        printf("}\n");
        fflush(stdout);
    }
    program_destroy(program);
    error_context_destroy(error_context);
    parser_destroy(parser);
    lexer_destroy(lexer);
    if (debug_enabled)
    {
        printf("[DEBUG] Exiting dump_ast_json\n");
        fflush(stdout);
    }
}

void dump_stmt_json(Stmt *stmt, int indent)
{
    if (!stmt)
    {
        printf("null");
        return;
    }

    print_json_indent(indent);
    printf("{\n");

    print_json_indent(indent + 2);
    printf("\"type\": \"");

    switch (stmt->type)
    {
    case STMT_EXPR:
        printf("expression_statement\",\n");
        print_json_indent(indent + 2);
        printf("\"expression\": ");
        dump_expr_json(stmt->data.expr.expression, indent + 4);
        break;

    case STMT_VAR_DECL:
        printf("variable_declaration\",\n");
        print_json_indent(indent + 2);
        printf("\"name\": \"%s\",\n", stmt->data.var_decl.name);
        print_json_indent(indent + 2);
        printf("\"data_type\": \"%s\",\n", data_type_to_string(stmt->data.var_decl.type));
        if (stmt->data.var_decl.initializer)
        {
            print_json_indent(indent + 2);
            printf("\"initializer\": ");
            dump_expr_json(stmt->data.var_decl.initializer, indent + 4);
            printf(",\n");
        }
        break;

    case STMT_ARRAY_DECL:
        printf("array_declaration\",\n");
        print_json_indent(indent + 2);
        printf("\"name\": \"%s\",\n", stmt->data.array_decl.name);
        print_json_indent(indent + 2);
        printf("\"element_type\": \"%s\",\n", data_type_to_string(stmt->data.array_decl.element_type));
        print_json_indent(indent + 2);
        printf("\"size\": %d", stmt->data.array_decl.size);
        if (stmt->data.array_decl.initializer)
        {
            printf(",\n");
            print_json_indent(indent + 2);
            printf("\"initializer\": ");
            dump_expr_json(stmt->data.array_decl.initializer, indent + 4);
        }
        break;

    case STMT_ASSIGNMENT:
        printf("assignment\",\n");
        print_json_indent(indent + 2);
        printf("\"target\": \"%s\",\n", stmt->data.assignment.name);
        print_json_indent(indent + 2);
        printf("\"value\": ");
        dump_expr_json(stmt->data.assignment.value, indent + 4);
        break;

    case STMT_ARRAY_ASSIGNMENT:
        printf("array_assignment\",\n");
        print_json_indent(indent + 2);
        printf("\"array\": ");
        dump_expr_json(stmt->data.array_assignment.array, indent + 4);
        printf(",\n");
        print_json_indent(indent + 2);
        printf("\"index\": ");
        dump_expr_json(stmt->data.array_assignment.index, indent + 4);
        printf(",\n");
        print_json_indent(indent + 2);
        printf("\"value\": ");
        dump_expr_json(stmt->data.array_assignment.value, indent + 4);
        break;

    case STMT_IF:
        printf("if_statement\",\n");
        print_json_indent(indent + 2);
        printf("\"condition\": ");
        dump_expr_json(stmt->data.if_stmt.condition, indent + 4);
        printf(",\n");
        print_json_indent(indent + 2);
        printf("\"then_branch\": ");
        dump_stmt_json(stmt->data.if_stmt.then_branch, indent + 4);
        if (stmt->data.if_stmt.else_branch)
        {
            printf(",\n");
            print_json_indent(indent + 2);
            printf("\"else_branch\": ");
            dump_stmt_json(stmt->data.if_stmt.else_branch, indent + 4);
        }
        break;

    case STMT_WHILE:
        printf("while_statement\",\n");
        print_json_indent(indent + 2);
        printf("\"condition\": ");
        dump_expr_json(stmt->data.while_stmt.condition, indent + 4);
        printf(",\n");
        print_json_indent(indent + 2);
        printf("\"body\": ");
        dump_stmt_json(stmt->data.while_stmt.body, indent + 4);
        break;

    case STMT_BREAK:
        printf("break_statement\"");
        break;

    case STMT_CONTINUE:
        printf("continue_statement\"");
        break;

    case STMT_RETURN:
        printf("return_statement\"");
        if (stmt->data.return_stmt.value)
        {
            printf(",\n");
            print_json_indent(indent + 2);
            printf("\"value\": ");
            dump_expr_json(stmt->data.return_stmt.value, indent + 4);
        }
        break;

    case STMT_PRINT:
        printf("print_statement\",\n");
        print_json_indent(indent + 2);
        printf("\"arguments\": [\n");
        for (size_t i = 0; i < stmt->data.print_stmt.args.size; i++)
        {
            Expr *arg = (Expr *)array_get(&stmt->data.print_stmt.args, i);
            dump_expr_json(arg, indent + 4);
            if (i < stmt->data.print_stmt.args.size - 1)
            {
                printf(",");
            }
            printf("\n");
        }
        print_json_indent(indent + 2);
        printf("]");
        break;

    case STMT_BLOCK:
        printf("block_statement\",\n");
        print_json_indent(indent + 2);
        printf("\"statements\": [\n");
        for (size_t i = 0; i < stmt->data.block.statements.size; i++)
        {
            Stmt *block_stmt = (Stmt *)array_get(&stmt->data.block.statements, i);
            dump_stmt_json(block_stmt, indent + 4);
            if (i < stmt->data.block.statements.size - 1)
            {
                printf(",");
            }
            printf("\n");
        }
        print_json_indent(indent + 2);
        printf("]");
        break;
    }

    printf("\n");
    print_json_indent(indent);
    printf("}");
}

void dump_expr_json(Expr *expr, int indent)
{
    if (!expr)
    {
        printf("null");
        return;
    }

    print_json_indent(indent);
    printf("{\n");

    print_json_indent(indent + 2);
    printf("\"type\": \"");

    switch (expr->type)
    {
    case EXPR_LITERAL:
        if (expr->data.literal.is_bool_literal)
        {
            printf("boolean_literal\",\n");
            print_json_indent(indent + 2);
            printf("\"value\": %s", expr->data.literal.value.bool_value ? "true" : "false");
        }
        else if (expr->data.literal.is_float_literal)
        {
            printf("float_literal\",\n");
            print_json_indent(indent + 2);
            printf("\"value\": %f", expr->data.literal.value.float_value);
        }
        else
        {
            printf("integer_literal\",\n");
            print_json_indent(indent + 2);
            printf("\"value\": %lld", expr->data.literal.value.number_value);
        }
        break;

    case EXPR_VARIABLE:
        printf("variable\",\n");
        print_json_indent(indent + 2);
        printf("\"name\": \"%s\"", expr->data.variable.name);
        break;

    case EXPR_BINARY:
        printf("binary_expression\",\n");
        print_json_indent(indent + 2);
        printf("\"operator\": \"%s\",\n", token_type_to_string(expr->data.binary.operator));
        print_json_indent(indent + 2);
        printf("\"left\": ");
        dump_expr_json(expr->data.binary.left, indent + 4);
        printf(",\n");
        print_json_indent(indent + 2);
        printf("\"right\": ");
        dump_expr_json(expr->data.binary.right, indent + 4);
        break;

    case EXPR_UNARY:
        printf("unary_expression\",\n");
        print_json_indent(indent + 2);
        printf("\"operator\": \"%s\",\n", token_type_to_string(expr->data.unary.operator));
        print_json_indent(indent + 2);
        printf("\"operand\": ");
        dump_expr_json(expr->data.unary.operand, indent + 4);
        break;

    case EXPR_CALL:
        printf("function_call\",\n");
        print_json_indent(indent + 2);
        printf("\"name\": \"%s\",\n", expr->data.call.name);
        print_json_indent(indent + 2);
        printf("\"arguments\": [\n");
        for (size_t i = 0; i < expr->data.call.args.size; i++)
        {
            Expr *arg = (Expr *)array_get(&expr->data.call.args, i);
            dump_expr_json(arg, indent + 4);
            if (i < expr->data.call.args.size - 1)
            {
                printf(",");
            }
            printf("\n");
        }
        print_json_indent(indent + 2);
        printf("]");
        break;

    case EXPR_GROUP:
        printf("group_expression\",\n");
        print_json_indent(indent + 2);
        printf("\"expression\": ");
        dump_expr_json(expr->data.group.expression, indent + 4);
        break;

    case EXPR_ARRAY_INDEX:
        printf("array_index\",\n");
        print_json_indent(indent + 2);
        printf("\"array\": ");
        dump_expr_json(expr->data.array_index.array, indent + 4);
        printf(",\n");
        print_json_indent(indent + 2);
        printf("\"index\": ");
        dump_expr_json(expr->data.array_index.index, indent + 4);
        break;

    case EXPR_STRING_INDEX:
        printf("string_index\",\n");
        print_json_indent(indent + 2);
        printf("\"string\": ");
        dump_expr_json(expr->data.string_index.string, indent + 4);
        printf(",\n");
        print_json_indent(indent + 2);
        printf("\"index\": ");
        dump_expr_json(expr->data.string_index.index, indent + 4);
        break;

    case EXPR_NULL_LITERAL:
        printf("null_literal\",\n");
        print_json_indent(indent + 2);
        printf("\"value\": null");
        break;
    }

    printf("\n");
    print_json_indent(indent);
    printf("}");
}

void print_json_indent(int indent)
{
    for (int i = 0; i < indent; i++)
    {
        printf(" ");
    }
}

bool compile_file(const char *input_filename, const char *output_filename, bool verbose, bool assembly_output)
{
    if (verbose)
    {
        printf("Using built-in specs.\n");
        printf("COLLECT_GCC=%s\n", "compiler.exe");
        printf("Target: %s\n", get_target_machine());
        printf("Configured with: --prefix=/usr/local --enable-languages=c\n");
        printf("Thread model: posix\n");
        printf("gcc version 1.0.0 (Twink Language Compiler)\n");
        printf("COLLECT_GCC_OPTIONS='-o' '%s'\n", output_filename);
        if (assembly_output)
        {
            printf(" %s %s %s\n", get_assembler_command(), output_filename, input_filename);
            printf(" %s %s %s\n", get_linker_command(), output_filename, output_filename);
        }
        else
        {
            printf(" %s %s %s\n", get_assembler_command(), output_filename, input_filename);
            printf(" %s %s %s\n", get_linker_command(), output_filename, output_filename);
        }
    }

    if (debug_enabled)
    {
        printf("[DEBUG] Entered compile_file\n");
        fflush(stdout);
    }
    char *source = read_file(input_filename);
    if (!source)
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Failed to read input file\n");
            fflush(stdout);
        }
        return false;
    }

    ErrorContext *error_context = error_context_create(input_filename, source);
    Error error;
    error_init(&error);

    Lexer *lexer = lexer_create(source, &error);
    if (error.type != ERROR_NONE)
    {
        error_context_add_error(error_context, error.type, SEVERITY_ERROR,
                                error.message, error.suggestion, error.line, error.column);
        error_init(&error);
    }

    Parser *parser = NULL;
    Program *program = NULL;
    if (lexer)
    {
        parser = parser_create(lexer, error_context);
        if (error.type != ERROR_NONE)
        {
            error_context_add_error(error_context, error.type, SEVERITY_ERROR,
                                    error.message, error.suggestion, error.line, error.column);
            error_init(&error);
        }
        else
        {
            program = parser_parse(parser);

            if (error.type != ERROR_NONE)
            {
                error_context_add_error(error_context, error.type, SEVERITY_ERROR,
                                        error.message, error.suggestion, error.line, error.column);
                error_init(&error);
            }
        }
    }

    SemanticAnalyzer *analyzer = NULL;
    if (program)
    {
        analyzer = semantic_create(program, error_context);
        if (error.type != ERROR_NONE)
        {
            error_context_add_error(error_context, error.type, SEVERITY_ERROR,
                                    error.message, error.suggestion, error.line, error.column);
            error_init(&error);
        }
        else
        {
            if (!semantic_analyze(analyzer))
            {
                // Semantic errors are already added to error_context by the analyzer
            }
        }
    }

    if (error_context_has_errors(error_context))
    {
        error_context_print_all(error_context);
        error_context_destroy(error_context);
        if (analyzer)
            semantic_destroy(analyzer);
        if (program)
            program_destroy(program);
        if (parser)
            parser_destroy(parser);
        if (lexer)
            lexer_destroy(lexer);
        safe_free(source);
        return false;
    }

    if (error_context->count > 0)
    {
        error_context_print_all(error_context);
        error_context->count = 0;
    }

    IRProgram *ir_program = NULL;
    if (analyzer && program)
    {
        ir_program = ir_generate(program, analyzer);
        if (!ir_program)
        {
            error_context_add_error(error_context, ERROR_CODEGEN, SEVERITY_ERROR,
                                    "Failed to generate intermediate representation",
                                    "Check for unsupported language constructs", 0, 0);
        }
    }

    if (error_context->count > 0)
    {
        error_context_print_all(error_context);
        error_context_destroy(error_context);
        if (ir_program)
            ir_program_destroy(ir_program);
        if (analyzer)
            semantic_destroy(analyzer);
        if (program)
            program_destroy(program);
        if (parser)
            parser_destroy(parser);
        if (lexer)
            lexer_destroy(lexer);
        safe_free(source);
        return false;
    }

    FILE *output_file = fopen(output_filename, "w");
    if (!output_file)
    {
        error_context_add_error(error_context, ERROR_CODEGEN, SEVERITY_ERROR,
                                "Cannot create output file",
                                "Check file permissions and disk space", 0, 0);
        error_context_print_all(error_context);
        error_context_destroy(error_context);
        if (ir_program)
            ir_program_destroy(ir_program);
        if (analyzer)
            semantic_destroy(analyzer);
        if (program)
            program_destroy(program);
        if (parser)
            parser_destroy(parser);
        if (lexer)
            lexer_destroy(lexer);
        safe_free(source);
        return false;
    }

    CodeGenerator *generator = NULL;
    bool success = false;

    if (assembly_output)
    {
        generator = codegenasm_create(ir_program, output_file, &error);
    }
    else
    {
        generator = codegen_create(ir_program, output_file, &error);
    }

    if (error.type != ERROR_NONE)
    {
        error_context_add_error(error_context, error.type, SEVERITY_ERROR,
                                error.message, error.suggestion, error.line, error.column);
    }
    else if (generator)
    {
        if (assembly_output)
        {
            success = codegenasm_generate(generator);
        }
        else
        {
            success = codegen_generate(generator);
        }

        if (!success)
        {
            error_context_add_error(error_context, ERROR_CODEGEN, SEVERITY_ERROR,
                                    "Code generation failed",
                                    "Check for unsupported language constructs", 0, 0);
        }
    }

    if (generator)
    {
        if (assembly_output)
        {
            codegenasm_destroy(generator);
        }
        else
        {
            codegen_destroy(generator);
        }
    }
    fclose(output_file);

    if (error_context->count > 0)
    {
        error_context_print_all(error_context);
        error_context_destroy(error_context);
        if (ir_program)
            ir_program_destroy(ir_program);
        if (analyzer)
            semantic_destroy(analyzer);
        if (program)
            program_destroy(program);
        if (parser)
            parser_destroy(parser);
        if (lexer)
            lexer_destroy(lexer);
        safe_free(source);
        return false;
    }

    error_context_destroy(error_context);
    if (ir_program)
        ir_program_destroy(ir_program);
    if (analyzer)
        semantic_destroy(analyzer);
    if (program)
        program_destroy(program);
    if (parser)
        parser_destroy(parser);
    if (lexer)
        lexer_destroy(lexer);
    safe_free(source);

    const char *output_type = assembly_output ? "assembly" : "C";
    printf("Successfully compiled '%s' to '%s' (%s)\n", input_filename, output_filename, output_type);
    fflush(stdout);

    if (debug_enabled)
    {
        printf("[DEBUG] Exiting compile_file\n");
        fflush(stdout);
    }
    return true;
}

// print_memory_usage_stats is already defined in src/common/common.c