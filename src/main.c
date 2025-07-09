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

#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <sys/utsname.h>
#elif defined(__APPLE__)
#include <sys/utsname.h>
#endif

// Function to get the target machine string dynamically
const char* get_target_machine(void) {
    static char machine_string[256];
    
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    
    const char* arch;
    switch (sysInfo.wProcessorArchitecture) {
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
    if (uname(&uts) == 0) {
            snprintf(machine_string, sizeof(machine_string), "%s-%s-linux-twink", 
            uts.machine, uts.sysname);
    } else {
        strcpy(machine_string, "x86_64-pc-linux-twink");
    }
    
#elif defined(__APPLE__)
    struct utsname uts;
    if (uname(&uts) == 0) {
            snprintf(machine_string, sizeof(machine_string), "%s-apple-darwin-twink", 
            uts.machine);
    } else {
        strcpy(machine_string, "x86_64-apple-darwin-twink");
    }
    
#else
    strcpy(machine_string, "unknown-unknown-unknown");
#endif
    
    return machine_string;
}

// Function to get the appropriate assembler command
const char* get_assembler_command(void) {
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

// Function to get the appropriate linker command
const char* get_linker_command(void) {
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

// Function to get the dynamic linker path
const char* get_dynamic_linker(void) {
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

bool has_tl_extension(const char* filename) {
    if (!filename) return false;
    
    size_t len = strlen(filename);
    if (len < 3) return false;  
    
    return (strcmp(filename + len - 3, ".tl") == 0);
}

bool has_c_extension(const char* filename) {
    if (!filename) return false;
    
    size_t len = strlen(filename);
    if (len < 2) return false;
    
    return (strcmp(filename + len - 2, ".c") == 0);
}

bool has_asm_extension(const char* filename) {
    if (!filename) return false;
    
    size_t len = strlen(filename);
    if (len < 2) return false;
    
    if (len >= 2 && strcmp(filename + len - 2, ".s") == 0) {
        return true;
    }
    
    if (len >= 4 && strcmp(filename + len - 4, ".asm") == 0) {
        return true;
    }
    
    return false;
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
    
    ErrorContext* error_context = error_context_create(filename, source);
    Parser* parser = parser_create(lexer, error_context);
    if (error.type != ERROR_NONE) {
        error_print(&error, filename);
        error_context_destroy(error_context);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    
    Program* program = parser_parse(parser);
    if (error.type != ERROR_NONE) {
        error_print(&error, filename);
        program_destroy(program);
        error_context_destroy(error_context);
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
    error_context_destroy(error_context);
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
    
    ErrorContext* error_context = error_context_create(filename, source);
    Parser* parser = parser_create(lexer, error_context);
    if (error.type != ERROR_NONE) {
        error_print(&error, filename);
        error_context_destroy(error_context);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    
    Program* program = parser_parse(parser);
    if (error.type != ERROR_NONE) {
        error_print(&error, filename);
        program_destroy(program);
        error_context_destroy(error_context);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    
    SemanticAnalyzer* analyzer = semantic_create(program, error_context);
    if (error.type != ERROR_NONE) {
        error_print(&error, filename);
        semantic_destroy(analyzer);
        program_destroy(program);
        error_context_destroy(error_context);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    
    if (!semantic_analyze(analyzer)) {
        semantic_destroy(analyzer);
        program_destroy(program);
        error_context_destroy(error_context);
        parser_destroy(parser);
        lexer_destroy(lexer);
        fflush(stdout);
        return;
    }
    
    IRProgram* ir_program = ir_generate(program, analyzer);
    if (!ir_program) {
        print_error("compiler", "failed to generate IR");
        semantic_destroy(analyzer);
        program_destroy(program);
        error_context_destroy(error_context);
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
    error_context_destroy(error_context);
    parser_destroy(parser);
    lexer_destroy(lexer);
    printf("[DEBUG] Exiting print_ir\n"); fflush(stdout);
}

bool compile_file(const char* input_filename, const char* output_filename, bool verbose, bool assembly_output) {
    if (verbose) {
        printf("Using built-in specs.\n");
        printf("COLLECT_GCC=%s\n", "compiler.exe");
        printf("Target: %s\n", get_target_machine());
        printf("Configured with: --prefix=/usr/local --enable-languages=c\n");
        printf("Thread model: posix\n");
        printf("gcc version 1.0.0 (Twink Language Compiler)\n");
        printf("COLLECT_GCC_OPTIONS='-o' '%s'\n", output_filename);
        if (assembly_output) {
            printf(" %s %s %s\n", get_assembler_command(), output_filename, input_filename);
            printf(" %s %s %s\n", get_linker_command(), output_filename, output_filename);
        } else {
            printf(" %s %s %s\n", get_assembler_command(), output_filename, input_filename);
            printf(" %s %s %s\n", get_linker_command(), output_filename, output_filename);
        }
    }
    
    printf("[DEBUG] Entered compile_file\n"); fflush(stdout);
    char* source = read_file(input_filename);
    if (!source) {
        printf("[DEBUG] Failed to read input file\n"); fflush(stdout);
        return false;
    }
    
    ErrorContext* error_context = error_context_create(input_filename, source);
    Error error;
    error_init(&error);
    
    Lexer* lexer = lexer_create(source, &error);
    if (error.type != ERROR_NONE) {
        error_context_add_error(error_context, error.type, SEVERITY_ERROR, 
                              error.message, error.suggestion, error.line, error.column);
        error_init(&error);
    }
    
    Parser* parser = NULL;
    Program* program = NULL;
    if (lexer) {
        parser = parser_create(lexer, error_context);
        if (error.type != ERROR_NONE) {
            error_context_add_error(error_context, error.type, SEVERITY_ERROR, 
                                  error.message, error.suggestion, error.line, error.column);
            error_init(&error);
        } else {
            program = parser_parse(parser);
            
            if (error.type != ERROR_NONE) {
                error_context_add_error(error_context, error.type, SEVERITY_ERROR, 
                                      error.message, error.suggestion, error.line, error.column);
                error_init(&error);
            }
        }
    }
    
    SemanticAnalyzer* analyzer = NULL;
    if (program) {  // Only do semantic analysis if we have a program (even with parse errors)
        analyzer = semantic_create(program, error_context);
        if (error.type != ERROR_NONE) {
            error_context_add_error(error_context, error.type, SEVERITY_ERROR, 
                                  error.message, error.suggestion, error.line, error.column);
            error_init(&error);
        } else {
            if (!semantic_analyze(analyzer)) {
                // Semantic errors are already added to error_context by the analyzer
            }
        }
    }
    
    if (error_context_has_errors(error_context)) {
        error_context_print_all(error_context);
        error_context_destroy(error_context);
        if (analyzer) semantic_destroy(analyzer);
        if (program) program_destroy(program);
        if (parser) parser_destroy(parser);
        if (lexer) lexer_destroy(lexer);
        safe_free(source);
        return false;
    }
    
    if (error_context->count > 0) {
        error_context_print_all(error_context);
        error_context->count = 0;
    }
    
    IRProgram* ir_program = NULL;
    if (analyzer && program) {
        ir_program = ir_generate(program, analyzer);
        if (!ir_program) {
            error_context_add_error(error_context, ERROR_CODEGEN, SEVERITY_ERROR, 
                                  "Failed to generate intermediate representation", 
                                  "Check for unsupported language constructs", 0, 0);
        }
    }
    
    if (error_context->count > 0) {
        error_context_print_all(error_context);
        error_context_destroy(error_context);
        if (ir_program) ir_program_destroy(ir_program);
        if (analyzer) semantic_destroy(analyzer);
        if (program) program_destroy(program);
        if (parser) parser_destroy(parser);
        if (lexer) lexer_destroy(lexer);
        safe_free(source);
        return false;
    }
    
    FILE* output_file = fopen(output_filename, "w");
    if (!output_file) {
        error_context_add_error(error_context, ERROR_CODEGEN, SEVERITY_ERROR, 
                              "Cannot create output file", 
                              "Check file permissions and disk space", 0, 0);
        error_context_print_all(error_context);
        error_context_destroy(error_context);
        if (ir_program) ir_program_destroy(ir_program);
        if (analyzer) semantic_destroy(analyzer);
        if (program) program_destroy(program);
        if (parser) parser_destroy(parser);
        if (lexer) lexer_destroy(lexer);
        safe_free(source);
        return false;
    }
    
    CodeGenerator* generator = NULL;
    bool success = false;
    
    if (assembly_output) {
        generator = codegenasm_create(ir_program, output_file, &error);
    } else {
        generator = codegen_create(ir_program, output_file, &error);
    }
    
    if (error.type != ERROR_NONE) {
        error_context_add_error(error_context, error.type, SEVERITY_ERROR, 
                              error.message, error.suggestion, error.line, error.column);
    } else if (generator) {
        if (assembly_output) {
            success = codegenasm_generate(generator);
        } else {
            success = codegen_generate(generator);
        }
        
        if (!success) {
            error_context_add_error(error_context, ERROR_CODEGEN, SEVERITY_ERROR, 
                                  "Code generation failed", 
                                  "Check for unsupported language constructs", 0, 0);
        }
    }
    
    if (generator) {
        if (assembly_output) {
            codegenasm_destroy(generator);
        } else {
            codegen_destroy(generator);
        }
    }
    fclose(output_file);
    
    if (error_context->count > 0) {
        error_context_print_all(error_context);
        error_context_destroy(error_context);
        if (ir_program) ir_program_destroy(ir_program);
        if (analyzer) semantic_destroy(analyzer);
        if (program) program_destroy(program);
        if (parser) parser_destroy(parser);
        if (lexer) lexer_destroy(lexer);
        safe_free(source);
        return false;
    }
    
    error_context_destroy(error_context);
    if (ir_program) ir_program_destroy(ir_program);
    if (analyzer) semantic_destroy(analyzer);
    if (program) program_destroy(program);
    if (parser) parser_destroy(parser);
    if (lexer) lexer_destroy(lexer);
    safe_free(source);
    
    const char* output_type = assembly_output ? "assembly" : "C";
    printf("Successfully compiled '%s' to '%s' (%s)\n", input_filename, output_filename, output_type);
    fflush(stdout);
    
    printf("[DEBUG] Exiting compile_file\n"); fflush(stdout);
    return true;
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
            printf("Spec strings for Twink Language Compiler:\n");
            printf("  *cpp: %s -E -undef -traditional\n", argv[0]);
            printf("  *cc1: %s -E -quiet -dumpbase %%B.dump -auxbase-strip %%s -o %%s\n", argv[0]);
            printf("  *as: %s\n", get_assembler_command());
            printf("  *ld: %s -dynamic-linker %s\n", get_linker_command(), get_dynamic_linker());
            printf("  *link: %s -E -Bstatic -o %%s %%s %%s %%s\n", argv[0]);
            return 0;
        } else if (strcmp(argv[i], "--dumpversion") == 0) {
            printf("1.0.0\n");
            return 0;
        } else if (strcmp(argv[i], "--dumpmachine") == 0) {
            printf("%s\n", get_target_machine());
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
        
        if (assembly_output) {
            if (!has_asm_extension(output_filename)) {
                print_error(argv[0], "assembly output requires .s or .asm extension");
                fprintf(stderr, "  %s\n", output_filename);
                fprintf(stderr, "  Use: %s %s -o %s.s --asm\n", argv[0], input_filename, 
                        output_filename[0] == '-' ? "output" : output_filename);
                safe_free(source);
                return 1;
            }
        } else {
            if (!has_c_extension(output_filename)) {
                print_error(argv[0], "C output requires .c extension");
                fprintf(stderr, "  %s\n", output_filename);
                fprintf(stderr, "  Use: %s %s -o %s.c\n", argv[0], input_filename, 
                        output_filename[0] == '-' ? "output" : output_filename);
                safe_free(source);
                return 1;
            }
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