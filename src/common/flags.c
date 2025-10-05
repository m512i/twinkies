#include "common/flags.h"
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
bool suppress_warnings = false;

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

void handle_module_mode(int *i, int argc, char *argv[], void *context)
{
    CompilerContext *ctx = (CompilerContext *)context;
    ctx->module_mode = true;

    if (*i + 1 < argc && argv[*i + 1][0] != '-')
    {
        ctx->module_output_dir = argv[++(*i)];
    }
    else
    {
        ctx->module_output_dir = "./build/modules";
    }
}

void handle_module_include_path(int *i, int argc, char *argv[], void *context)
{
    CompilerContext *ctx = (CompilerContext *)context;
    if (*i + 1 < argc)
    {
        char *path = malloc(strlen(argv[*i + 1]) + 1);
        strcpy(path, argv[++(*i)]);
        array_push(&ctx->module_include_paths, path);
    }
    else
    {
        print_error(argv[0], "missing include path after -I");
        exit(1);
    }
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
    char *filename = string_copy(argv[*i]);
    array_push(&ctx->input_filenames, filename);
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
    {"--modules", handle_module_mode, "Enable module compilation mode"},
    {"-I", handle_module_include_path, "Add include path for modules"},
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
    printf("Usage: %s <input_file> [input_file2] ... -o <output_file>\n", program_name);
    printf("       %s <input_file> [input_file2] ... -o <output_file> --asm\n", program_name);
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
    printf("      Multiple input files are supported for multi-file compilation.\n");
    fflush(stdout);
}