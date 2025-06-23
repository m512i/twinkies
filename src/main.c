#include "../include/common.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/ast.h"
#include "../include/semantic.h"
#include "../include/ir.h"
#include "../include/codegen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool has_tl_extension(const char* filename) {
    if (!filename) return false;
    
    size_t len = strlen(filename);
    if (len < 3) return false;  
    
    return (strcmp(filename + len - 3, ".tl") == 0);
}

void print_usage(const char* program_name) {
    printf("Usage: %s <input_file> -o <output_file>\n", program_name);
    printf("       %s <input_file> -o <output_file> --asm\n", program_name);
    printf("       %s <input_file> --tokens\n", program_name);
    printf("       %s <input_file> --ast\n", program_name);
    printf("       %s <input_file> --ir\n", program_name);
    printf("\n");
    printf("Options:\n");
    printf("  -o <output_file>    Specify output file (C or assembly)\n");
    printf("  --asm               Generate assembly code instead of C\n");
    printf("  --tokens            Print tokens from lexer\n");
    printf("  --ast               Print AST from parser\n");
    printf("  --ir                Print IR from semantic analysis\n");
    printf("  --v                 Display the programs invoked by the compiler\n");
    printf("  --dumpspecs         Display all of the built in spec strings\n");
    printf("  --dumpversion       Display the version of the compiler\n");
    printf("  --dumpmachine       Display the compiler's target processor\n");
    printf("  --help              Show this help message\n");
    printf("\n");
    printf("Note: Only files with .tl extension can be compiled.\n");
    fflush(stdout);
}

char* read_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        print_error("compiler", "cannot open file");
        fprintf(stderr, "  %s\n", filename);
        fflush(stderr);
        return NULL;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* buffer = safe_malloc(file_size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }
    
    size_t bytes_read = fread(buffer, 1, file_size, file);
    buffer[bytes_read] = '\0';
    
    fclose(file);
    return buffer;
}

void print_tokens(const char* source, const char* filename) {
    printf("[DEBUG] Entered print_tokens\n"); fflush(stdout);
    Error error;
    error_init(&error);
    
    Lexer* lexer = lexer_create(source, &error);
    if (error.type != ERROR_NONE) {
        error_print(&error, filename);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    
    printf("Tokens for %s:\n", filename);
    printf("================\n");
    fflush(stdout);
    
    Token token;
    do {
        token = lexer_next_token(lexer);
        token_print(&token);
        token_destroy(&token);
        fflush(stdout);
    } while (token.type != TOKEN_EOF);
    
    lexer_destroy(lexer);
    printf("[DEBUG] Exiting print_tokens\n"); fflush(stdout);
}

void print_ast(const char* source, const char* filename) {
    printf("[DEBUG] Entered print_ast\n"); fflush(stdout);
    Error error;
    error_init(&error);
    
    Lexer* lexer = lexer_create(source, &error);
    if (error.type != ERROR_NONE) {
        error_print(&error, filename);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    
    Parser* parser = parser_create(lexer, &error);
    if (error.type != ERROR_NONE) {
        error_print(&error, filename);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    
    Program* program = parser_parse(parser);
    if (error.type != ERROR_NONE) {
        error_print(&error, filename);
        program_destroy(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    
    printf("AST for %s:\n", filename);
    printf("============\n");
    fflush(stdout);
    program_print(program);
    fflush(stdout);
    
    program_destroy(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    printf("[DEBUG] Exiting print_ast\n"); fflush(stdout);
}

void print_ir(const char* source, const char* filename) {
    printf("[DEBUG] Entered print_ir\n"); fflush(stdout);
    Error error;
    error_init(&error);
    
    Lexer* lexer = lexer_create(source, &error);
    if (error.type != ERROR_NONE) {
        error_print(&error, filename);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    
    Parser* parser = parser_create(lexer, &error);
    if (error.type != ERROR_NONE) {
        error_print(&error, filename);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    
    Program* program = parser_parse(parser);
    if (error.type != ERROR_NONE) {
        error_print(&error, filename);
        program_destroy(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    
    SemanticAnalyzer* analyzer = semantic_create(program, &error);
    if (error.type != ERROR_NONE) {
        error_print(&error, filename);
        semantic_destroy(analyzer);
        program_destroy(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    
    if (!semantic_analyze(analyzer)) {
        error_print(&error, filename);
        semantic_destroy(analyzer);
        program_destroy(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    
    IRProgram* ir_program = ir_generate(program);
    if (!ir_program) {
        print_error("compiler", "failed to generate IR");
        semantic_destroy(analyzer);
        program_destroy(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    
    printf("IR for %s:\n", filename);
    printf("===========\n");
    fflush(stdout);
    ir_program_print(ir_program);
    fflush(stdout);
    
    ir_program_destroy(ir_program);
    semantic_destroy(analyzer);
    program_destroy(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    printf("[DEBUG] Exiting print_ir\n"); fflush(stdout);
}

bool compile_file(const char* input_filename, const char* output_filename, bool verbose, bool assembly_output) {
    if (verbose) {
        printf("Using built-in specs.\n");
        printf("COLLECT_GCC=%s\n", "compiler.exe");
        printf("Target: x86_64-pc-windows-msvc\n");
        printf("Configured with: --prefix=/usr/local --enable-languages=c\n");
        printf("Thread model: posix\n");
        printf("gcc version 1.0.0 (Tiny Language Compiler)\n");
        printf("COLLECT_GCC_OPTIONS='-o' '%s'\n", output_filename);
        if (assembly_output) {
            printf(" /usr/bin/as --64 -o %s %s\n", output_filename, input_filename);
            printf(" /usr/bin/ld -m i386pep -o %s %s\n", output_filename, output_filename);
        } else {
            printf(" /usr/bin/as --32 -o %s %s\n", output_filename, input_filename);
            printf(" /usr/bin/ld -m i386pe -o %s %s\n", output_filename, output_filename);
        }
    }
    
    printf("[DEBUG] Entered compile_file\n"); fflush(stdout);
    char* source = read_file(input_filename);
    if (!source) {
        printf("[DEBUG] Failed to read input file\n"); fflush(stdout);
        return false;
    }
    
    Error error;
    error_init(&error);
    
    Lexer* lexer = lexer_create(source, &error);
    if (error.type != ERROR_NONE) {
        error_print(&error, input_filename);
        safe_free(source);
        printf("[DEBUG] Lexer error\n"); fflush(stdout);
        return false;
    }
    
    Parser* parser = parser_create(lexer, &error);
    if (error.type != ERROR_NONE) {
        error_print(&error, input_filename);
        parser_destroy(parser);
        lexer_destroy(lexer);
        safe_free(source);
        printf("[DEBUG] Parser error\n"); fflush(stdout);
        return false;
    }
    
    Program* program = parser_parse(parser);
    if (error.type != ERROR_NONE) {
        error_print(&error, input_filename);
        program_destroy(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        safe_free(source);
        printf("[DEBUG] Program parse error\n"); fflush(stdout);
        return false;
    }
    
    SemanticAnalyzer* analyzer = semantic_create(program, &error);
    if (error.type != ERROR_NONE) {
        error_print(&error, input_filename);
        semantic_destroy(analyzer);
        program_destroy(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        safe_free(source);
        printf("[DEBUG] Semantic analyzer error\n"); fflush(stdout);
        return false;
    }
    
    if (!semantic_analyze(analyzer)) {
        error_print(&error, input_filename);
        semantic_destroy(analyzer);
        program_destroy(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        safe_free(source);
        printf("[DEBUG] Semantic analyze failed\n"); fflush(stdout);
        return false;
    }
    
    IRProgram* ir_program = ir_generate(program);
    if (!ir_program) {
        print_error("compiler", "failed to generate IR");
        semantic_destroy(analyzer);
        program_destroy(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return false;
    }
    
    FILE* output_file = fopen(output_filename, "w");
    if (!output_file) {
        print_error("compiler", "cannot create output file");
        fprintf(stderr, "  %s\n", output_filename);
        ir_program_destroy(ir_program);
        semantic_destroy(analyzer);
        program_destroy(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        safe_free(source);
        printf("[DEBUG] Output file creation failed\n"); fflush(stdout);
        return false;
    }
    
    CodeGenerator* generator;
    if (assembly_output) {
        generator = codegenasm_create(ir_program, output_file, &error);
    } else {
        generator = codegen_create(ir_program, output_file, &error);
    }
    
    if (error.type != ERROR_NONE) {
        error_print(&error, input_filename);
        if (assembly_output) {
            codegenasm_destroy(generator);
        } else {
            codegen_destroy(generator);
        }
        fclose(output_file);
        ir_program_destroy(ir_program);
        semantic_destroy(analyzer);
        program_destroy(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        safe_free(source);
        printf("[DEBUG] Codegen error\n"); fflush(stdout);
        return false;
    }
    
    bool success;
    if (assembly_output) {
        success = codegenasm_generate(generator);
    } else {
        success = codegen_generate(generator);
    }
    printf("[DEBUG] Codegen_generate returned %d\n", success); fflush(stdout);
    
    if (assembly_output) {
        codegenasm_destroy(generator);
    } else {
        codegen_destroy(generator);
    }
    fclose(output_file);
    ir_program_destroy(ir_program);
    semantic_destroy(analyzer);
    program_destroy(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    safe_free(source);
    
    if (success) {
        const char* output_type = assembly_output ? "assembly" : "C";
        printf("Successfully compiled '%s' to '%s' (%s)\n", input_filename, output_filename, output_type);
        fflush(stdout);
    }
    
    printf("[DEBUG] Exiting compile_file\n"); fflush(stdout);
    return success;
}

int main(int argc, char* argv[]) {
    printf("[DEBUG] Entered main\n"); fflush(stdout);
    if (argc < 2) {
        print_fatal_error(argv[0], "no input files");
        fprintf(stderr, "compilation terminated.\n");
        fflush(stderr);
        return 1;
    }
    
    const char* input_filename = NULL;
    const char* output_filename = NULL;
    bool print_tokens_flag = false;
    bool print_ast_flag = false;
    bool print_ir_flag = false;
    bool verbose_flag = false;
    bool assembly_output = false;
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_usage(argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--dumpspecs") == 0) {
            printf("Spec strings for Tiny Language Compiler:\n");
            printf("  *cpp: %s -E -undef -traditional\n", argv[0]);
            printf("  *cc1: %s -E -quiet -dumpbase %%B.dump -auxbase-strip %%s -o %%s\n", argv[0]);
            printf("  *as: as --32\n");
            printf("  *ld: ld -m elf_i386 -dynamic-linker /lib/ld-linux.so.2\n");
            printf("  *link: %s -E -Bstatic -o %%s %%s %%s %%s\n", argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--dumpversion") == 0) {
            printf("1.0.0\n");
            return 0;
        } else if (strcmp(argv[i], "--dumpmachine") == 0) {
            printf("x86_64-pc-windows-msvc\n");
            return 0;
        } else if (strcmp(argv[i], "--v") == 0) {
            verbose_flag = true;
        } else if (strcmp(argv[i], "--tokens") == 0) {
            print_tokens_flag = true;
        } else if (strcmp(argv[i], "--ast") == 0) {
            print_ast_flag = true;
        } else if (strcmp(argv[i], "--ir") == 0) {
            print_ir_flag = true;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_filename = argv[++i];
            } else {
                print_error(argv[0], "missing output filename after -o");
                return 1;
            }
        } else if (strcmp(argv[i], "--asm") == 0) {
            assembly_output = true;
        } else if (!input_filename) {
            input_filename = argv[i];
        } else {
            print_error(argv[0], "unknown argument");
            fprintf(stderr, "  %s\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }
    
    if (!input_filename) {
        print_fatal_error(argv[0], "no input files");
        fprintf(stderr, "compilation terminated.\n");
        fflush(stderr);
        return 1;
    }
    
    if (!has_tl_extension(input_filename)) {
        print_error(argv[0], "only files with .tl extension can be compiled");
        fprintf(stderr, "  %s\n", input_filename);
        return 1;
    }
    
    char* source = read_file(input_filename);
    if (!source) {
        printf("[DEBUG] Failed to read source in main\n"); fflush(stdout);
        return 1;
    }
    
    if (print_tokens_flag) {
        print_tokens(source, input_filename);
    } else if (print_ast_flag) {
        print_ast(source, input_filename);
    } else if (print_ir_flag) {
        print_ir(source, input_filename);
    } else {
        if (!output_filename) {
            print_error(argv[0], "output file not specified (use -o)");
            print_usage(argv[0]);
            safe_free(source);
            return 1;
        }
        
        if (!compile_file(input_filename, output_filename, verbose_flag, assembly_output)) {
            safe_free(source);
            printf("[DEBUG] compile_file returned false\n"); fflush(stdout);
            return 1;
        }
    }
    
    safe_free(source);
    printf("[DEBUG] Exiting main\n"); fflush(stdout);
    return 0;
} 