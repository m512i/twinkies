#include "common.h"
#include "frontend/lexer.h"
#include "frontend/parser.h"
#include "frontend/ast.h"
#include "analysis/semantic.h"
#include "backend/ir.h"
#include "backend/codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <sys/utsname.h>
#elif defined(__APPLE__)
#include <sys/utsname.h>
#endif

bool debug_enabled = false;

void print_usage(const char *program_name);
const char *get_target_machine(void);
const char *get_assembler_command(void);
const char *get_linker_command(void);
const char *get_dynamic_linker(void);
void print_tokens(const char *source, const char *filename);
void print_ast(const char *source, const char *filename);
void print_ir(const char *source, const char *filename);
void dump_ast_json(const char *source, const char *filename);
void dump_stmt_json(Stmt *stmt, int indent);
void dump_expr_json(Expr *expr, int indent);
void print_json_indent(int indent);

typedef struct
{
    const char *name;
    void (*handler)(int *i, int argc, char *argv[], void *context);
    const char *description;
} Command;

typedef struct
{
    const char *input_filename;
    const char *output_filename;
    bool print_tokens_flag;
    bool print_ast_flag;
    bool print_ir_flag;
    bool dump_ast_flag;
    bool verbose_flag;
    bool assembly_output;
    bool suppress_warnings;
    bool memory_stats_flag;
} CompilerContext;

void handle_help(int *i, int argc, char *argv[], void *context)
{
    (void)i;
    (void)argc;
    (void)argv;
    (void)context;
    print_usage(argv[0]);
    exit(0);
}

void handle_dumpspecs(int *i, int argc, char *argv[], void *context)
{
    (void)i;
    (void)argc;
    (void)argv;
    (void)context;
    printf("Spec strings for Twink Language Compiler:\n");
    printf("  *cpp: %s -E -undef -traditional\n", argv[0]);
    printf("  *cc1: %s -E -quiet -dumpbase %%B.dump -auxbase-strip %%s -o %%s\n", argv[0]);
    printf("  *as: %s\n", get_assembler_command());
    printf("  *ld: %s -dynamic-linker %s\n", get_linker_command(), get_dynamic_linker());
    printf("  *link: %s -E -Bstatic -o %%s %%s %%s %%s\n", argv[0]);
    exit(0);
}

void handle_dumpversion(int *i, int argc, char *argv[], void *context)
{
    (void)i;
    (void)argc;
    (void)argv;
    (void)context;
    printf("1.0.0\n");
    exit(0);
}

void handle_dumpmachine(int *i, int argc, char *argv[], void *context)
{
    (void)i;
    (void)argc;
    (void)argv;
    (void)context;
    printf("%s\n", get_target_machine());
    exit(0);
}

void handle_verbose(int *i, int argc, char *argv[], void *context)
{
    (void)i;
    (void)argc;
    (void)argv;
    CompilerContext *ctx = (CompilerContext *)context;
    ctx->verbose_flag = true;
}

void handle_tokens(int *i, int argc, char *argv[], void *context)
{
    (void)i;
    (void)argc;
    (void)argv;
    CompilerContext *ctx = (CompilerContext *)context;
    ctx->print_tokens_flag = true;
}

void handle_ast(int *i, int argc, char *argv[], void *context)
{
    (void)i;
    (void)argc;
    (void)argv;
    CompilerContext *ctx = (CompilerContext *)context;
    ctx->print_ast_flag = true;
}

void handle_ir(int *i, int argc, char *argv[], void *context)
{
    (void)i;
    (void)argc;
    (void)argv;
    CompilerContext *ctx = (CompilerContext *)context;
    ctx->print_ir_flag = true;
}

void handle_dump_ast_json(int *i, int argc, char *argv[], void *context)
{
    (void)i;
    (void)argc;
    (void)argv;
    CompilerContext *ctx = (CompilerContext *)context;
    ctx->dump_ast_flag = true;
}

void handle_no_warnings(int *i, int argc, char *argv[], void *context)
{
    (void)i;
    (void)argc;
    (void)argv;
    CompilerContext *ctx = (CompilerContext *)context;
    ctx->suppress_warnings = true;
}

void handle_memory_stats(int *i, int argc, char *argv[], void *context)
{
    (void)i;
    (void)argc;
    (void)argv;
    CompilerContext *ctx = (CompilerContext *)context;
    ctx->memory_stats_flag = true;
}

void handle_output(int *i, int argc, char *argv[], void *context)
{
    CompilerContext *ctx = (CompilerContext *)context;
    if (*i + 1 < argc)
    {
        ctx->output_filename = argv[++(*i)];
    }
    else
    {
        print_error(argv[0], "missing output filename after -o");
        exit(1);
    }
}

void handle_asm(int *i, int argc, char *argv[], void *context)
{
    (void)i;
    (void)argc;
    (void)argv;
    CompilerContext *ctx = (CompilerContext *)context;
    ctx->assembly_output = true;
}

void handle_input_file(int *i, int argc, char *argv[], void *context)
{
    (void)argc;
    CompilerContext *ctx = (CompilerContext *)context;
    if (!ctx->input_filename)
    {
        ctx->input_filename = argv[*i];
    }
    else
    {
        print_error(argv[0], "unknown argument");
        fprintf(stderr, "  %s\n", argv[*i]);
        print_usage(argv[0]);
        exit(1);
    }
}

void handle_debug(int *i, int argc, char *argv[], void *context)
{
    (void)i;
    (void)argc;
    (void)argv;
    (void)context;
    debug_enabled = true;
}

static const Command commands[] = {
    {"--help", handle_help, "Show this help message"},
    {"--dumpspecs", handle_dumpspecs, "Display all of the built in spec strings"},
    {"--dumpversion", handle_dumpversion, "Display the version of the compiler"},
    {"--dumpmachine", handle_dumpmachine, "Display the compiler's target processor"},
    {"--v", handle_verbose, "Display the programs invoked by the compiler"},
    {"--tokens", handle_tokens, "Print tokens from lexer"},
    {"--ast", handle_ast, "Print AST from parser"},
    {"--ir", handle_ir, "Print IR from semantic analysis"},
    {"--dump-ast-json", handle_dump_ast_json, "Dump AST in JSON format"},
    {"--no-warnings", handle_no_warnings, "Suppress warning messages"},
    {"-o", handle_output, "Specify output file"},
    {"--asm", handle_asm, "Generate assembly code instead of C"},
    {"--debug", handle_debug, "Enable debug output"},
    {"--memory", handle_memory_stats, "Show memory usage statistics"},
    {NULL, handle_input_file, "Input file"}};

void process_argument(int *i, int argc, char *argv[], CompilerContext *context)
{
    for (size_t j = 0; j < sizeof(commands) / sizeof(commands[0]); j++)
    {
        if (commands[j].name == NULL)
        {
            commands[j].handler(i, argc, argv, context);
            return;
        }
        if (strcmp(argv[*i], commands[j].name) == 0)
        {
            commands[j].handler(i, argc, argv, context);
            return;
        }
    }

    print_error(argv[0], "unknown argument");
    fprintf(stderr, "  %s\n", argv[*i]);
    print_usage(argv[0]);
    exit(1);
}

bool suppress_warnings = false;

const char *get_target_machine(void)
{
    static char machine_string[256];

#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    const char *arch;
    switch (sysInfo.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_AMD64:
        arch = "x86_64";
        break;
    case PROCESSOR_ARCHITECTURE_ARM:
        arch = "arm";
        break;
    case PROCESSOR_ARCHITECTURE_ARM64:
        arch = "aarch64";
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        arch = "i386";
        break;
    default:
        arch = "unknown";
        break;
    }

    snprintf(machine_string, sizeof(machine_string), "%s-pc-windows-twink", arch);

#elif defined(__linux__)
    struct utsname uts;
    if (uname(&uts) == 0)
    {
        snprintf(machine_string, sizeof(machine_string), "%s-%s-linux-twink",
                 uts.machine, uts.sysname);
    }
    else
    {
        strcpy(machine_string, "x86_64-pc-linux-twink");
    }

#elif defined(__APPLE__)
    struct utsname uts;
    if (uname(&uts) == 0)
    {
        snprintf(machine_string, sizeof(machine_string), "%s-apple-darwin-twink",
                 uts.machine);
    }
    else
    {
        strcpy(machine_string, "x86_64-apple-darwin-twink");
    }

#else
    strcpy(machine_string, "unknown-unknown-unknown");
#endif

    return machine_string;
}

const char *get_assembler_command(void)
{
#ifdef _WIN32
    return "ml64 /c /Fo";
#elif defined(__linux__)
    return "as --64 -o";
#elif defined(__APPLE__)
    return "as -o";
#else
    return "as -o";
#endif
}

const char *get_linker_command(void)
{
#ifdef _WIN32
    return "link /OUT:";
#elif defined(__linux__)
    return "ld -m elf_x86_64 -o";
#elif defined(__APPLE__)
    return "ld -o";
#else
    return "ld -o";
#endif
}

const char *get_dynamic_linker(void)
{
#ifdef _WIN32
    return "kernel32.dll";
#elif defined(__linux__)
    return "/lib64/ld-linux-x86-64.so.2";
#elif defined(__APPLE__)
    return "/usr/lib/dyld";
#else
    return "/lib/ld.so";
#endif
}

bool has_tl_extension(const char *filename)
{
    if (!filename)
        return false;

    size_t len = strlen(filename);
    if (len < 3)
        return false;

    return (strcmp(filename + len - 3, ".tl") == 0);
}

bool has_c_extension(const char *filename)
{
    if (!filename)
        return false;

    size_t len = strlen(filename);
    if (len < 2)
        return false;

    return (strcmp(filename + len - 2, ".c") == 0);
}

bool has_asm_extension(const char *filename)
{
    if (!filename)
        return false;

    size_t len = strlen(filename);
    if (len < 2)
        return false;

    if (len >= 2 && strcmp(filename + len - 2, ".s") == 0)
    {
        return true;
    }

    if (len >= 4 && strcmp(filename + len - 4, ".asm") == 0)
    {
        return true;
    }

    return false;
}

void print_usage(const char *program_name)
{
    printf("Usage: %s <input_file> -o <output_file>\n", program_name);
    printf("       %s <input_file> -o <output_file> --asm\n", program_name);
    printf("       %s <input_file> --tokens\n", program_name);
    printf("       %s <input_file> --ast\n", program_name);
    printf("       %s <input_file> --ir\n", program_name);
    printf("       %s <input_file> --memory\n", program_name);
    printf("\n");
    printf("Options:\n");

    for (size_t i = 0; i < sizeof(commands) / sizeof(commands[0]); i++)
    {
        if (commands[i].name != NULL)
        {
            printf("  %-20s %s\n", commands[i].name, commands[i].description);
        }
    }
    printf("\n");
    printf("Note: Only files with .tl extension can be compiled.\n");
    fflush(stdout);
}

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
        printf("\"value\": ");
        dump_expr_json(stmt->data.print_stmt.value, indent + 4);
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

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--debug") == 0)
        {
            debug_enabled = true;
            break;
        }
    }

    printf("[DEBUG] Entered main\n");
    fflush(stdout);
    if (argc < 2)
    {
        print_fatal_error(argv[0], "no input files");
        fprintf(stderr, "compilation terminated.\n");
        fflush(stderr);
        if (argc > 1)
        {
            for (int i = 1; i < argc; i++)
            {
                if (strcmp(argv[i], "--memory") == 0)
                {
                    print_memory_usage_stats();
                    break;
                }
            }
        }
        return 1;
    }

    CompilerContext context = {NULL, NULL, false, false, false, false, false, false, false, false};

    for (int i = 1; i < argc; i++)
    {
        process_argument(&i, argc, argv, &context);
    }

    if (!context.input_filename)
    {
        print_fatal_error(argv[0], "no input files");
        fprintf(stderr, "compilation terminated.\n");
        fflush(stderr);
        if (context.memory_stats_flag)
            print_memory_usage_stats();
        return 1;
    }

    if (!has_tl_extension(context.input_filename))
    {
        print_error(argv[0], "only files with .tl extension can be compiled");
        fprintf(stderr, "  %s\n", context.input_filename);
        if (context.memory_stats_flag)
            print_memory_usage_stats();
        return 1;
    }

    char *source = read_file(context.input_filename);
    if (!source)
    {
        printf("[DEBUG] Failed to read source in main\n");
        fflush(stdout);
        if (context.memory_stats_flag)
            print_memory_usage_stats();
        return 1;
    }

    if (context.print_tokens_flag)
    {
        print_tokens(source, context.input_filename);
        if (context.memory_stats_flag)
            print_memory_usage_stats();
    }
    else if (context.print_ast_flag)
    {
        print_ast(source, context.input_filename);
        if (context.memory_stats_flag)
            print_memory_usage_stats();
    }
    else if (context.print_ir_flag)
    {
        print_ir(source, context.input_filename);
        if (context.memory_stats_flag)
            print_memory_usage_stats();
    }
    else if (context.dump_ast_flag)
    {
        dump_ast_json(source, context.input_filename);
        if (context.memory_stats_flag)
            print_memory_usage_stats();
    }
    else
    {
        suppress_warnings = context.suppress_warnings;

        if (!context.output_filename)
        {
            print_error(argv[0], "output file not specified (use -o)");
            print_usage(argv[0]);
            safe_free(source);
            if (context.memory_stats_flag)
                print_memory_usage_stats();
            return 1;
        }

        if (context.assembly_output)
        {
            if (!has_asm_extension(context.output_filename))
            {
                print_error(argv[0], "assembly output requires .s or .asm extension");
                fprintf(stderr, "  %s\n", context.output_filename);
                fprintf(stderr, "  Use: %s %s -o %s.s --asm\n", argv[0], context.input_filename,
                        context.output_filename[0] == '-' ? "output" : context.output_filename);
                safe_free(source);
                if (context.memory_stats_flag)
                    print_memory_usage_stats();
                return 1;
            }
        }
        else
        {
            if (!has_c_extension(context.output_filename))
            {
                print_error(argv[0], "C output requires .c extension");
                fprintf(stderr, "  %s\n", context.output_filename);
                fprintf(stderr, "  Use: %s %s -o %s.c\n", argv[0], context.input_filename,
                        context.output_filename[0] == '-' ? "output" : context.output_filename);
                safe_free(source);
                if (context.memory_stats_flag)
                    print_memory_usage_stats();
                return 1;
            }
        }

        if (!compile_file(context.input_filename, context.output_filename, context.verbose_flag, context.assembly_output))
        {
            safe_free(source);
            printf("[DEBUG] compile_file returned false\n");
            fflush(stdout);
            if (context.memory_stats_flag)
                print_memory_usage_stats();
            return 1;
        }
    }

    safe_free(source);
    printf("[DEBUG] Exiting main\n");
    fflush(stdout);
    if (context.memory_stats_flag)
        print_memory_usage_stats();
    return 0;
}