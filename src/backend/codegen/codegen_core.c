#include "backend/codegen/codegen_core.h"
#include "backend/codegen/codegen_strategy.h"
#include "backend/codegen/codegen_instruction_handlers.h"
#include "backend/codegen/codegen_ffi.h"
#include "backend/codegen/codegen_c_writer.h"
#include "common/flags.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

extern bool debug_enabled;

CodeGenerator *codegen_core_create(IRProgram *ir_program, Program *program, FILE *output_file, Error *error)
{
    CodeGenerator *generator = safe_malloc(sizeof(CodeGenerator));
    generator->ir_program = ir_program;
    generator->program = program;
    generator->output_file = output_file;
    generator->error = error;
    generator->indent_level = 0;
    generator->temp_counter = 0;
    generator->temp_map = hashtable_create(16);
    generator->var_set = hashtable_create(16);
    generator->array_info = hashtable_create(16);
    generator->variable_types = hashtable_create(16);
    generator->param_count = 0;
    generator->strategy = c_codegen_strategy_create();
    generator->current_function_name = NULL;
    generator->current_function_return_type = TYPE_INT;
    generator->epilogue_label[0] = '\0';
    generator->declared_temps = hashtable_create(16);
    return generator;
}

void codegen_core_destroy(CodeGenerator *generator)
{
    if (!generator)
        return;

    if (generator->strategy) {
        generator->strategy->destroy(generator->strategy);
    }
    
    hashtable_destroy(generator->temp_map);
    hashtable_destroy(generator->var_set);
    hashtable_destroy(generator->array_info);
    hashtable_destroy(generator->variable_types);
    hashtable_destroy(generator->declared_temps);
    safe_free(generator);
}

bool codegen_core_generate(CodeGenerator *generator)
{
    if (debug_enabled)
    {
        printf("[DEBUG] codegen_generate: Starting code generation\n");
        printf("[DEBUG] codegen_generate: IR program has %zu functions\n", generator->ir_program->functions.size);
    }

    if (!generator->strategy) {
        codegen_core_error(generator, "No code generation strategy set");
        return false;
    }

    generator->strategy->generate_header(generator);
    generator->strategy->generate_program(generator);
    return true;
}

void codegen_core_generate_program(CodeGenerator *generator)
{
    if (debug_enabled)
    {
        printf("[DEBUG] codegen_generate_program: Starting with %zu functions\n", generator->ir_program->functions.size);
    }

    for (size_t i = 0; i < generator->ir_program->functions.size; i++)
    {
        IRFunction *func = (IRFunction *)array_get(&generator->ir_program->functions, i);
        if (debug_enabled)
        {
            printf("[DEBUG] codegen_generate_program: Processing function %s with %zu instructions\n", func->name, func->instructions.size);
        }
        generator->strategy->generate_function(generator, func);
    }

    bool has_main = false;
    for (size_t i = 0; i < generator->ir_program->functions.size; i++)
    {
        IRFunction *func = (IRFunction *)array_get(&generator->ir_program->functions, i);
        if (string_equal(func->name, "main"))
        {
            has_main = true;
            break;
        }
    }

    if (!has_main)
    {
        if (debug_enabled)
        {
            printf("[DEBUG] codegen_generate_program: No main function found, adding default main\n");
        }
        codegen_c_writer_write_main_function(generator);
    }
}

void codegen_core_generate_function(CodeGenerator *generator, IRFunction *func)
{
    generator->strategy->generate_function(generator, func);
}

void codegen_core_write_indent(CodeGenerator *generator)
{
    for (int i = 0; i < generator->indent_level; i++)
    {
        fprintf(generator->output_file, "    ");
    }
}

void codegen_core_write_line(CodeGenerator *generator, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    codegen_core_write_indent(generator);
    vfprintf(generator->output_file, format, args);
    fprintf(generator->output_file, "\n");

    va_end(args);
}

void codegen_core_error(CodeGenerator *generator, const char *message)
{
    if (generator->error)
    {
        error_set(generator->error, ERROR_CODEGEN, message, 0, 0);
    }
}

const char *codegen_core_get_c_type_string(DataType type)
{
    switch (type)
    {
    case TYPE_INT:
        return "int64_t";
    case TYPE_BOOL:
        return "bool";
    case TYPE_FLOAT:
        return "float";
    case TYPE_DOUBLE:
        return "double";
    case TYPE_STRING:
        return "char*";
    case TYPE_ARRAY:
        return "int64_t"; // This will be overridden by the actual element type
    case TYPE_NULL:
        return "void*";
    default:
        return "int64_t";
    }
}

bool codegen_core_is_array_variable(CodeGenerator *generator, const char *var_name)
{
    for (size_t i = 0; i < generator->ir_program->functions.size; i++)
    {
        IRFunction *func = (IRFunction *)array_get(&generator->ir_program->functions, i);
        for (size_t j = 0; j < func->instructions.size; j++)
        {
            IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, j);
            if (instr->opcode == IR_ARRAY_DECL)
            {
                if (instr->result && instr->result->type == IR_OP_VAR &&
                    string_equal(instr->result->data.var_name, var_name))
                {
                    return true;
                }
            }
            if (instr->opcode == IR_ARRAY_LOAD || instr->opcode == IR_ARRAY_STORE)
            {
                if (instr->arg1 && instr->arg1->type == IR_OP_VAR &&
                    string_equal(instr->arg1->data.var_name, var_name))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

int codegen_core_get_array_size(CodeGenerator *generator, const char *var_name)
{
    for (size_t i = 0; i < generator->ir_program->functions.size; i++)
    {
        IRFunction *func = (IRFunction *)array_get(&generator->ir_program->functions, i);
        for (size_t j = 0; j < func->instructions.size; j++)
        {
            IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, j);
            if (instr->opcode == IR_ARRAY_DECL)
            {
                if (instr->result && instr->result->type == IR_OP_VAR &&
                    string_equal(instr->result->data.var_name, var_name))
                {
                    return instr->result->array_size;
                }
            }
            if (instr->opcode == IR_ARRAY_LOAD || instr->opcode == IR_ARRAY_STORE)
            {
                if (instr->arg1 && instr->arg1->type == IR_OP_VAR &&
                    string_equal(instr->arg1->data.var_name, var_name))
                {
                    return instr->arg1->array_size;
                }
            }
        }
    }
    return -1;
}
