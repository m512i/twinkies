#include "common/common.h"
#include "common/flags.h"
#include "common/utils.h"
#include "frontend/lexer/lexer.h"
#include "frontend/parser/parser.h"
#include "frontend/ast/ast.h"
#include "analysis/semantic/semantic.h"
#include "backend/ir/ir.h"
#include "backend/codegen.h"
#include "modules/modules.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

    CompilerContext context = {0};
    array_init(&context.input_filenames, 4);
    array_init(&context.module_include_paths, 4);

    for (int i = 1; i < argc; i++)
    {
        process_argument(&i, argc, argv, &context);
    }

    if (context.input_filenames.size == 0)
    {
        print_fatal_error(argv[0], "no input files");
        fprintf(stderr, "compilation terminated.\n");
        fflush(stderr);
        if (context.memory_stats_flag)
            print_memory_usage_stats();
        return 1;
    }

    const char *main_input_file = (const char *)array_get(&context.input_filenames, 0);

    if (!has_tl_extension(main_input_file))
    {
        print_error(argv[0], "only files with .tl extension can be compiled");
        fprintf(stderr, "  %s\n", main_input_file);
        if (context.memory_stats_flag)
            print_memory_usage_stats();
        return 1;
    }

    char *source = read_file(main_input_file);
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
        print_tokens(source, main_input_file);
        if (context.memory_stats_flag)
            print_memory_usage_stats();
    }
    else if (context.print_ast_flag)
    {
        print_ast(source, main_input_file);
        if (context.memory_stats_flag)
            print_memory_usage_stats();
    }
    else if (context.print_ir_flag)
    {
        print_ir(source, main_input_file);
        if (context.memory_stats_flag)
            print_memory_usage_stats();
    }
    else if (context.dump_ast_flag)
    {
        dump_ast_json(source, main_input_file);
        if (context.memory_stats_flag)
            print_memory_usage_stats();
    }
    else
    {
        suppress_warnings = context.suppress_warnings;

        if (context.module_mode)
        {
            if (!compile_module_system(main_input_file, context.output_filename,
                                       context.verbose_flag, context.module_output_dir,
                                       &context.module_include_paths))
            {
                safe_free(source);
                if (context.memory_stats_flag)
                    print_memory_usage_stats();
                return 1;
            }
        }
        else
        {
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
                    fprintf(stderr, "  Use: %s %s -o %s.s --asm\n", argv[0], main_input_file,
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
                    fprintf(stderr, "  Use: %s %s -o %s.c\n", argv[0], main_input_file,
                            context.output_filename[0] == '-' ? "output" : context.output_filename);
                    safe_free(source);
                    if (context.memory_stats_flag)
                        print_memory_usage_stats();
                    return 1;
                }
            }

            if (context.input_filenames.size > 1)
            {
                if (!compile_multiple_files(&context.input_filenames, context.output_filename, context.verbose_flag, context.assembly_output))
                {
                    safe_free(source);
                    printf("[DEBUG] compile_multiple_files returned false\n");
                    fflush(stdout);
                    if (context.memory_stats_flag)
                        print_memory_usage_stats();
                    return 1;
                }
            }
            else
            {
                if (!compile_file(main_input_file, context.output_filename, context.verbose_flag, context.assembly_output))
                {
                    safe_free(source);
                    printf("[DEBUG] compile_file returned false\n");
                    fflush(stdout);
                    if (context.memory_stats_flag)
                        print_memory_usage_stats();
                    return 1;
                }
            }
        }

        safe_free(source);

        for (size_t i = 0; i < context.input_filenames.size; i++)
        {
            char *filename = (char *)array_get(&context.input_filenames, i);
            safe_free(filename);
        }
        array_free(&context.input_filenames);

        printf("[DEBUG] Exiting main\n");
        fflush(stdout);
        if (context.memory_stats_flag)
            print_memory_usage_stats();
        return 0;
    }

    safe_free(source);
    printf("[DEBUG] Exiting main\n");
    fflush(stdout);
    if (context.memory_stats_flag)
        print_memory_usage_stats();
    return 0;
}