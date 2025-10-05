#ifndef FLAGS_H
#define FLAGS_H

#include "common/common.h"

extern bool debug_enabled;
extern bool suppress_warnings;

typedef void (*CommandHandler)(int *i, int argc, char *argv[], void *context);

typedef struct
{
    const char *name;
    CommandHandler handler;
    const char *description;
} Command;

typedef struct
{
    DynamicArray input_filenames;
    const char *output_filename;
    bool print_tokens_flag;
    bool print_ast_flag;
    bool print_ir_flag;
    bool dump_ast_flag;
    bool verbose_flag;
    bool assembly_output;
    bool suppress_warnings;
    bool memory_stats_flag;
    bool module_mode;
    char *module_output_dir;
    DynamicArray module_include_paths;
} CompilerContext;

void handle_help(int *i, int argc, char *argv[], void *context);
void handle_dumpspecs(int *i, int argc, char *argv[], void *context);
void handle_dumpversion(int *i, int argc, char *argv[], void *context);
void handle_dumpmachine(int *i, int argc, char *argv[], void *context);
void handle_verbose(int *i, int argc, char *argv[], void *context);
void handle_tokens(int *i, int argc, char *argv[], void *context);
void handle_ast(int *i, int argc, char *argv[], void *context);
void handle_ir(int *i, int argc, char *argv[], void *context);
void handle_dump_ast_json(int *i, int argc, char *argv[], void *context);
void handle_no_warnings(int *i, int argc, char *argv[], void *context);
void handle_memory_stats(int *i, int argc, char *argv[], void *context);
void handle_module_mode(int *i, int argc, char *argv[], void *context);
void handle_module_include_path(int *i, int argc, char *argv[], void *context);
void handle_output(int *i, int argc, char *argv[], void *context);
void handle_asm(int *i, int argc, char *argv[], void *context);
void handle_input_file(int *i, int argc, char *argv[], void *context);
void handle_debug(int *i, int argc, char *argv[], void *context);
void process_argument(int *i, int argc, char *argv[], CompilerContext *context);
void print_usage(const char *program_name);

const char *get_target_machine(void);
const char *get_assembler_command(void);
const char *get_linker_command(void);
const char *get_dynamic_linker(void);

bool has_tl_extension(const char *filename);
bool has_c_extension(const char *filename);
bool has_asm_extension(const char *filename);

#endif