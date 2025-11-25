#include "common/utils.h"
#include "common/flags.h"
#include "modules/modules.h"
#include "frontend/lexer/lexer.h"
#include "frontend/parser/parser.h"
#include "frontend/ast/ast.h"
#include "frontend/ast/aststmt.h"
#include "analysis/semantic/semantic.h"
#include "backend/ir/ir.h"
#include "backend/codegen/codegen.h"
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

    case STMT_INCLUDE:
        printf("include_directive\",\n");
        print_json_indent(indent + 2);
        printf("\"path\": \"%s\",\n", stmt->data.include.path);
        print_json_indent(indent + 2);
        printf("\"type\": \"%s\"", stmt->data.include.type == INCLUDE_SYSTEM ? "system" : "local");
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

bool compile_multiple_files(DynamicArray *input_filenames, const char *output_filename, bool verbose, bool assembly_output)
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
            printf(" %s %s", get_assembler_command(), output_filename);
            for (size_t i = 0; i < input_filenames->size; i++)
            {
                const char *filename = (const char *)array_get(input_filenames, i);
                printf(" %s", filename);
            }
            printf("\n");
            printf(" %s %s %s\n", get_linker_command(), output_filename, output_filename);
        }
        else
        {
            printf(" %s %s", get_assembler_command(), output_filename);
            for (size_t i = 0; i < input_filenames->size; i++)
            {
                const char *filename = (const char *)array_get(input_filenames, i);
                printf(" %s", filename);
            }
            printf("\n");
            printf(" %s %s %s\n", get_linker_command(), output_filename, output_filename);
        }
    }

    if (debug_enabled)
    {
        printf("[DEBUG] Entered compile_multiple_files with %zu files\n", input_filenames->size);
        fflush(stdout);
    }

    Program *combined_program = program_create();
    ErrorContext *combined_error_context = error_context_create("combined", "");

    for (size_t i = 0; i < input_filenames->size; i++)
    {
        const char *input_filename = (const char *)array_get(input_filenames, i);

        if (debug_enabled)
        {
            printf("[DEBUG] Processing file %zu: %s\n", i, input_filename);
            fflush(stdout);
        }

        char *source = read_file(input_filename);
        if (!source)
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Failed to read input file: %s\n", input_filename);
                fflush(stdout);
            }
            program_destroy(combined_program);
            error_context_destroy(combined_error_context);
            return false;
        }

        ErrorContext *file_error_context = error_context_create(input_filename, source);
        Error error;
        error_init(&error);

        Lexer *lexer = lexer_create(source, &error);
        if (error.type != ERROR_NONE)
        {
            error_context_add_error(file_error_context, error.type, SEVERITY_ERROR,
                                    error.message, error.suggestion, error.line, error.column);
            error_init(&error);
        }

        Parser *parser = NULL;
        Program *file_program = NULL;
        if (lexer)
        {
            parser = parser_create(lexer, file_error_context);
            if (error.type != ERROR_NONE)
            {
                error_context_add_error(file_error_context, error.type, SEVERITY_ERROR,
                                        error.message, error.suggestion, error.line, error.column);
                error_init(&error);
            }
            else
            {
                file_program = parser_parse(parser);

                if (error.type != ERROR_NONE)
                {
                    error_context_add_error(file_error_context, error.type, SEVERITY_ERROR,
                                            error.message, error.suggestion, error.line, error.column);
                    error_init(&error);
                }
            }
        }

        if (file_program)
        {
            for (size_t j = 0; j < file_program->functions.size; j++)
            {
                Function *func = (Function *)array_get(&file_program->functions, j);
                Function *func_copy = function_create(func->name, func->return_type);

                for (size_t k = 0; k < func->params.size; k++)
                {
                    Parameter *param = (Parameter *)array_get(&func->params, k);
                    Parameter *param_copy = safe_malloc(sizeof(Parameter));
                    param_copy->name = string_copy(param->name);
                    param_copy->type = param->type;
                    array_push(&func_copy->params, param_copy);
                }

                if (func->body)
                {
                    func_copy->body = stmt_copy(func->body);
                }

                array_push(&combined_program->functions, func_copy);
            }

            for (size_t j = 0; j < file_program->includes.size; j++)
            {
                char *include = (char *)array_get(&file_program->includes, j);
                array_push(&combined_program->includes, string_copy(include));
            }

            program_destroy(file_program);
        }

        for (size_t j = 0; j < file_error_context->count; j++)
        {
            Error *file_error = &file_error_context->errors[j];
            error_context_add_error(combined_error_context, file_error->type, file_error->severity,
                                    file_error->message, file_error->suggestion, file_error->line, file_error->column);
        }

        error_context_destroy(file_error_context);
        if (parser)
            parser_destroy(parser);
        if (lexer)
            lexer_destroy(lexer);
        safe_free(source);
    }

    SemanticAnalyzer *analyzer = NULL;
    if (combined_program)
    {
        analyzer = semantic_create(combined_program, combined_error_context);
        if (!semantic_analyze(analyzer))
        {
            // Semantic errors are already added to error_context by the analyzer
        }
    }

    if (error_context_has_errors(combined_error_context))
    {
        error_context_print_all(combined_error_context);
        error_context_destroy(combined_error_context);
        if (analyzer)
            semantic_destroy(analyzer);
        if (combined_program)
            program_destroy(combined_program);
        return false;
    }

    if (combined_error_context->count > 0)
    {
        error_context_print_all(combined_error_context);
        combined_error_context->count = 0;
    }

    IRProgram *ir_program = NULL;
    if (analyzer && combined_program)
    {
        ir_program = ir_generate(combined_program, analyzer);
        if (!ir_program)
        {
            error_context_add_error(combined_error_context, ERROR_CODEGEN, SEVERITY_ERROR,
                                    "Failed to generate intermediate representation",
                                    "Check for unsupported language constructs", 0, 0);
        }
    }

    if (combined_error_context->count > 0)
    {
        error_context_print_all(combined_error_context);
        error_context_destroy(combined_error_context);
        if (ir_program)
            ir_program_destroy(ir_program);
        if (analyzer)
            semantic_destroy(analyzer);
        if (combined_program)
            program_destroy(combined_program);
        return false;
    }

    FILE *output_file = fopen(output_filename, "w");
    if (!output_file)
    {
        error_context_add_error(combined_error_context, ERROR_CODEGEN, SEVERITY_ERROR,
                                "Cannot create output file",
                                "Check file permissions and disk space", 0, 0);
        error_context_print_all(combined_error_context);
        error_context_destroy(combined_error_context);
        if (ir_program)
            ir_program_destroy(ir_program);
        if (analyzer)
            semantic_destroy(analyzer);
        if (combined_program)
            program_destroy(combined_program);
        return false;
    }

    CodeGenerator *generator = NULL;
    bool success = false;
    Error error;
    error_init(&error);

    if (assembly_output)
    {
        generator = codegenasm_create(ir_program, output_file, &error);
    }
    else
    {
        generator = codegen_create(ir_program, combined_program, output_file, &error);
    }

    if (error.type != ERROR_NONE)
    {
        error_context_add_error(combined_error_context, error.type, SEVERITY_ERROR,
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
            error_context_add_error(combined_error_context, ERROR_CODEGEN, SEVERITY_ERROR,
                                    "Code generation failed",
                                    "Check for unsupported language constructs", 0, 0);
        }
    }

    if (generator)
    {
        codegen_destroy(generator);
    }
    fclose(output_file);

    if (combined_error_context->count > 0)
    {
        error_context_print_all(combined_error_context);
        error_context_destroy(combined_error_context);
        if (ir_program)
            ir_program_destroy(ir_program);
        if (analyzer)
            semantic_destroy(analyzer);
        if (combined_program)
            program_destroy(combined_program);
        return false;
    }

    error_context_destroy(combined_error_context);
    if (ir_program)
        ir_program_destroy(ir_program);
    if (analyzer)
        semantic_destroy(analyzer);
    if (combined_program)
        program_destroy(combined_program);

    printf("Successfully compiled %zu files to '%s'\n", input_filenames->size, output_filename);
    fflush(stdout);

    return true;
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
        else if (debug_enabled)
        {
            printf("[DEBUG] compile_file: IR program created with %zu functions\n", ir_program->functions.size);
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
        generator = codegen_create(ir_program, program, output_file, &error);
    }

    if (error.type != ERROR_NONE)
    {
        error_context_add_error(error_context, error.type, SEVERITY_ERROR,
                                error.message, error.suggestion, error.line, error.column);
    }
    else if (generator)
    {
        if (debug_enabled)
        {
            printf("[DEBUG] compile_file: Code generator created, starting generation\n");
        }

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
        else if (debug_enabled)
        {
            printf("[DEBUG] compile_file: Code generation completed successfully\n");
        }
    }
    else if (debug_enabled)
    {
        printf("[DEBUG] compile_file: Failed to create code generator\n");
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

bool compile_module_system(const char *input_filename, const char *output_filename, bool verbose,
                           const char *module_output_dir, DynamicArray *include_paths)
{
    if (verbose)
    {
        printf("Compiling with module system\n");
        printf("Input file: %s\n", input_filename);
        printf("Output file: %s\n", output_filename);
        printf("Module output directory: %s\n", module_output_dir);
    }

    ModuleManager *manager = module_manager_create();
    if (verbose)
    {
        manager->verbose = true;
    }

    if (module_output_dir)
    {
        manager->output_directory = string_copy(module_output_dir);
    }

    module_manager_add_include_path(manager, ".");
    module_manager_add_include_path(manager, "./build");

    if (include_paths)
    {
        for (size_t i = 0; i < include_paths->size; i++)
        {
            char *path = (char *)array_get(include_paths, i);
            module_manager_add_include_path(manager, path);
        }
    }

    char *source = read_file(input_filename);
    if (!source)
    {
        printf("Error: Cannot read input file '%s'\n", input_filename);
        module_manager_destroy(manager);
        return false;
    }

    ErrorContext *error_context = error_context_create(input_filename, source);
    Error error = {ERROR_NONE, SEVERITY_ERROR, "", "", 0, 0, "", 0, 0};

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
        }
    }

    if (error_context_has_errors(error_context))
    {
        error_context_print_all(error_context);
        error_context_destroy(error_context);
        if (program)
            program_destroy(program);
        if (parser)
            parser_destroy(parser);
        if (lexer)
            lexer_destroy(lexer);
        safe_free(source);
        module_manager_destroy(manager);
        return false;
    }

    if (program)
    {
        for (size_t i = 0; i < program->includes.size; i++)
        {
            Stmt *stmt = (Stmt *)array_get(&program->includes, i);
            if (stmt->type == STMT_INCLUDE)
            {
                const char *include_path = stmt->data.include.path;
                IncludeType include_type = stmt->data.include.type;

                if (verbose)
                {
                    printf("Processing include: %s (type: %s)\n",
                           include_path,
                           include_type == INCLUDE_SYSTEM ? "system" : "local");
                }

                if (verbose)
                {
                    printf("[DEBUG] Resolving include: %s (type: %s)\n",
                           include_path,
                           include_type == INCLUDE_SYSTEM ? "system" : "local");
                }

                char *resolved_path = module_manager_resolve_include(manager, include_path, include_type);
                if (!resolved_path)
                {
                    printf("Error: Cannot resolve include '%s'\n", include_path);
                    error_context_add_error(error_context, ERROR_PARSER, SEVERITY_ERROR,
                                            "Cannot resolve include",
                                            "Check if the file exists and is in the include path",
                                            stmt->line, stmt->column);
                    continue;
                }

                if (verbose)
                {
                    printf("[DEBUG] Resolved include '%s' to '%s'\n", include_path, resolved_path);
                }

                char *module_name = get_module_name_from_path(resolved_path);
                Module *module = module_create(module_name, resolved_path);

                if (verbose)
                {
                    printf("Created module: %s from %s\n", module_name, resolved_path);
                }

                if (!module_manager_add_module(manager, module))
                {
                    printf("Warning: Module %s already exists\n", module_name);
                    module_destroy(module);
                    safe_free(module_name);
                    safe_free(resolved_path);
                    continue;
                }

                if (verbose)
                {
                    printf("[DEBUG] Compiling module: %s\n", module_name);
                }

                if (!module_compile_source(manager, module))
                {
                    printf("Error: Failed to compile module %s\n", module_name);
                    error_context_add_error(error_context, ERROR_PARSER, SEVERITY_ERROR,
                                            "Failed to compile included module",
                                            "Check for syntax errors in the included file",
                                            stmt->line, stmt->column);
                }
                else if (verbose)
                {
                    printf("[DEBUG] Successfully compiled module: %s\n", module_name);
                }

                safe_free(module_name);
                safe_free(resolved_path);
            }
        }
    }

    if (error_context_has_errors(error_context))
    {
        error_context_print_all(error_context);
        error_context_destroy(error_context);
        if (program)
            program_destroy(program);
        if (parser)
            parser_destroy(parser);
        if (lexer)
            lexer_destroy(lexer);
        safe_free(source);
        module_manager_destroy(manager);
        return false;
    }

    SemanticAnalyzer *analyzer = NULL;
    if (program)
    {
        analyzer = semantic_create(program, error_context);

        for (size_t i = 0; i < manager->modules.size; i++)
        {
            Module *module = (Module *)array_get(&manager->modules, i);
            if (verbose)
            {
                printf("Processing module: %s\n", module->name);
                printf("Module has %zu exported symbols\n", module->exported_symbols.size);
                printf("Module has %zu functions in AST\n", module->ast ? module->ast->functions.size : 0);
            }

            if (module->ast)
            {
                for (size_t j = 0; j < module->exported_symbols.size; j++)
                {
                    char *symbol_name = (char *)array_get(&module->exported_symbols, j);
                    if (verbose)
                    {
                        printf("Processing exported symbol: %s\n", symbol_name);
                    }

                    for (size_t k = 0; k < module->ast->functions.size; k++)
                    {
                        Function *func = (Function *)array_get(&module->ast->functions, k);
                        if (strcmp(func->name, symbol_name) == 0)
                        {
                            if (verbose)
                            {
                                printf("Found function %s in module %s, adding to global scope\n", symbol_name, module->name);
                            }
                            semantic_add_global_function_with_params(analyzer, func);
                            break;
                        }
                    }
                }
            }
        }

        if (!semantic_analyze(analyzer))
        {
            // Semantic errors are already added to error_context by the analyzer
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
        module_manager_destroy(manager);
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
        ir_program = ir_generate_with_modules(program, analyzer, manager);
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
        module_manager_destroy(manager);
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
        module_manager_destroy(manager);
        return false;
    }

    CodeGenerator *generator = NULL;
    bool success = false;

    generator = codegen_create(ir_program, program, output_file, &error);

    if (error.type != ERROR_NONE)
    {
        error_context_add_error(error_context, error.type, SEVERITY_ERROR,
                                error.message, error.suggestion, error.line, error.column);
    }
    else if (generator)
    {
        success = codegen_generate(generator);

        if (!success)
        {
            error_context_add_error(error_context, ERROR_CODEGEN, SEVERITY_ERROR,
                                    "Code generation failed",
                                    "Check for unsupported language constructs", 0, 0);
        }
    }

    if (generator)
    {
        codegen_destroy(generator);
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
        module_manager_destroy(manager);
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
    module_manager_destroy(manager);

    printf("Successfully compiled '%s' to '%s' with module system\n", input_filename, output_filename);
    fflush(stdout);

    return true;
}

// print_memory_usage_stats is already defined in src/common/common.c