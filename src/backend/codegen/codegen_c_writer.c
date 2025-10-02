#include "backend/codegen_c_writer.h"
#include "backend/codegen_ffi.h"
#include "flags.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

extern bool debug_enabled;

static void escape_string_for_c(const char *input, FILE *output) {
    while (*input) {
        switch (*input) {
            case '\n':
                fprintf(output, "\\n");
                break;
            case '\t':
                fprintf(output, "\\t");
                break;
            case '\r':
                fprintf(output, "\\r");
                break;
            case '\\':
                fprintf(output, "\\\\");
                break;
            case '"':
                fprintf(output, "\\\"");
                break;
            default:
                fputc(*input, output);
                break;
        }
        input++;
    }
}

void codegen_c_writer_write_header(CodeGenerator *generator)
{
    fprintf(generator->output_file, "#include <stdio.h>\n");
    fprintf(generator->output_file, "#include <stdlib.h>\n");
    fprintf(generator->output_file, "#include <stdint.h>\n");
    fprintf(generator->output_file, "#include <stdbool.h>\n");
    fprintf(generator->output_file, "#include <inttypes.h>\n");
    fprintf(generator->output_file, "#include <string.h>\n");
    
    if (generator->program && generator->program->ffi_functions.size > 0)
    {
        fprintf(generator->output_file, "#ifdef _WIN32\n");
        fprintf(generator->output_file, "#include <windows.h>\n");
        fprintf(generator->output_file, "#endif\n");
    }
    
    fprintf(generator->output_file, "\n");

    codegen_c_writer_write_runtime_functions(generator);

    codegen_ffi_write_declarations(generator, generator->program);
    codegen_ffi_write_loading(generator, generator->program);

    for (size_t i = 0; i < generator->ir_program->functions.size; i++)
    {
        IRFunction *func = (IRFunction *)array_get(&generator->ir_program->functions, i);
        if (!string_equal(func->name, "main"))
        {
            DataType return_type = func->return_type;

            const char *return_type_str = codegen_c_writer_get_c_type_string(return_type);
            fprintf(generator->output_file, "%s %s(", return_type_str, func->name);

            if (func->params.size == 0)
            {
                fprintf(generator->output_file, "void");
            }
            else
            {
                for (size_t j = 0; j < func->params.size; j++)
                {
                    if (j > 0)
                        fprintf(generator->output_file, ", ");
                    IROperand *param = (IROperand *)array_get(&func->params, j);
                    const char *param_type = codegen_c_writer_get_c_type_string(param->data_type);
                    fprintf(generator->output_file, "%s %s", param_type, param->data.var_name);
                }
            }

            fprintf(generator->output_file, ");\n");
        }
    }
    fprintf(generator->output_file, "\n");
}

void codegen_c_writer_write_runtime_functions(CodeGenerator *generator)
{
    fprintf(generator->output_file, "char* __tl_concat(const char* a, const char* b) {\n");
    fprintf(generator->output_file, "    if (!a) a = \"\";\n");
    fprintf(generator->output_file, "    if (!b) b = \"\";\n");
    fprintf(generator->output_file, "    size_t len_a = strlen(a);\n");
    fprintf(generator->output_file, "    size_t len_b = strlen(b);\n");
    fprintf(generator->output_file, "    char* result = (char*)malloc(len_a + len_b + 1);\n");
    fprintf(generator->output_file, "    if (!result) { fprintf(stderr, \"Out of memory\\n\"); exit(1); }\n");
    fprintf(generator->output_file, "    strcpy(result, a);\n");
    fprintf(generator->output_file, "    strcat(result, b);\n");
    fprintf(generator->output_file, "    return result;\n");
    fprintf(generator->output_file, "}\n\n");

    fprintf(generator->output_file, "int64_t __tl_strlen(const char* str) {\n");
    fprintf(generator->output_file, "    if (!str) return 0;\n");
    fprintf(generator->output_file, "    return (int64_t)strlen(str);\n");
    fprintf(generator->output_file, "}\n\n");

    fprintf(generator->output_file, "char* __tl_substr(const char* str, int64_t start, int64_t len) {\n");
    fprintf(generator->output_file, "    if (!str) return strdup(\"\");\n");
    fprintf(generator->output_file, "    size_t str_len = strlen(str);\n");
    fprintf(generator->output_file, "    if (start < 0 || start >= str_len || len < 0) {\n");
    fprintf(generator->output_file, "        return strdup(\"\");\n");
    fprintf(generator->output_file, "    }\n");
    fprintf(generator->output_file, "    if (start + len > str_len) {\n");
    fprintf(generator->output_file, "        len = str_len - start;\n");
    fprintf(generator->output_file, "    }\n");
    fprintf(generator->output_file, "    char* result = (char*)malloc(len + 1);\n");
    fprintf(generator->output_file, "    if (!result) { fprintf(stderr, \"Out of memory\\n\"); exit(1); }\n");
    fprintf(generator->output_file, "    strncpy(result, str + start, len);\n");
    fprintf(generator->output_file, "    result[len] = '\\0';\n");
    fprintf(generator->output_file, "    return result;\n");
    fprintf(generator->output_file, "}\n\n");

    fprintf(generator->output_file, "int64_t __tl_strcmp(const char* a, const char* b) {\n");
    fprintf(generator->output_file, "    if (!a && !b) return 0;\n");
    fprintf(generator->output_file, "    if (!a) return -1;\n");
    fprintf(generator->output_file, "    if (!b) return 1;\n");
    fprintf(generator->output_file, "    // Unix-style implementation that returns actual character difference\n");
    fprintf(generator->output_file, "    while (*a != '\\0' && *a == *b) {\n");
    fprintf(generator->output_file, "        a++;\n");
    fprintf(generator->output_file, "        b++;\n");
    fprintf(generator->output_file, "    }\n");
    fprintf(generator->output_file, "    return (int64_t)((unsigned char)*a - (unsigned char)*b);\n");
    fprintf(generator->output_file, "}\n\n");

    fprintf(generator->output_file, "char* __tl_char_at(const char* str, int64_t index) {\n");
    fprintf(generator->output_file, "    if (!str) return strdup(\"\");\n");
    fprintf(generator->output_file, "    size_t len = strlen(str);\n");
    fprintf(generator->output_file, "    if (index < 0 || index >= len) {\n");
    fprintf(generator->output_file, "        return strdup(\"\");\n");
    fprintf(generator->output_file, "    }\n");
    fprintf(generator->output_file, "    char* result = (char*)malloc(2);\n");
    fprintf(generator->output_file, "    if (!result) { fprintf(stderr, \"Out of memory\\n\"); exit(1); }\n");
    fprintf(generator->output_file, "    result[0] = str[index];\n");
    fprintf(generator->output_file, "    result[1] = '\\0';\n");
    fprintf(generator->output_file, "    return result;\n");
    fprintf(generator->output_file, "}\n\n");
}

void codegen_c_writer_write_function_header(CodeGenerator *generator, IRFunction *func)
{
    if (string_equal(func->name, "main"))
    {
        fprintf(generator->output_file, "int main(void) {\n");
    }
    else
    {
        DataType return_type = func->return_type;

        const char *return_type_str = codegen_c_writer_get_c_type_string(return_type);
        fprintf(generator->output_file, "%s %s(", return_type_str, func->name);

        if (func->params.size == 0)
        {
            fprintf(generator->output_file, "void");
        }
        else
        {
            for (size_t i = 0; i < func->params.size; i++)
            {
                if (i > 0)
                    fprintf(generator->output_file, ", ");
                IROperand *param = (IROperand *)array_get(&func->params, i);
                const char *param_type = codegen_c_writer_get_c_type_string(param->data_type);
                fprintf(generator->output_file, "%s %s", param_type, param->data.var_name);
            }
        }

        fprintf(generator->output_file, ") {\n");
    }

    generator->indent_level++;

    if (string_equal(func->name, "main") && generator->program && generator->program->ffi_functions.size > 0)
    {
        codegen_c_writer_write_indent(generator);
        fprintf(generator->output_file, "load_ffi_functions();\n");
    }

    for (size_t j = 0; j < func->instructions.size; j++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, j);
        if (instr->opcode == IR_ARRAY_DECL)
        {
            if (instr->result && instr->result->type == IR_OP_VAR)
            {
                codegen_c_writer_write_indent(generator);
                const char *c_type = codegen_c_writer_get_c_type_string(instr->result->data_type);
                fprintf(generator->output_file, "%s %s[%d];\n", c_type, instr->result->data.var_name, instr->result->array_size);
            }
        }
        else if (instr->opcode == IR_VAR_DECL)
        {
            if (instr->result && instr->result->type == IR_OP_VAR)
            {
                codegen_c_writer_write_indent(generator);
                const char *c_type = codegen_c_writer_get_c_type_string(instr->result->data_type);
                fprintf(generator->output_file, "%s %s;\n", c_type, instr->result->data.var_name);
            }
        }
    }
    for (int i = 0; i < func->temp_counter; i++)
    {
        char temp_name[32];
        snprintf(temp_name, sizeof(temp_name), "temp_%d", i);
        DataType temp_type = TYPE_INT;

        if (debug_enabled)
        {
            printf("[DEBUG] codegen: Determining type for temp_%d in function %s\n", i, func->name);
        }

        bool found_as_result = false;
        for (size_t j = 0; j < func->instructions.size; j++)
        {
            IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, j);

            if (instr->result && instr->result->type == IR_OP_TEMP &&
                instr->result->data.temp_id == i)
            {
                temp_type = instr->result->data_type;
                found_as_result = true;
                if (debug_enabled)
                {
                    printf("[DEBUG] codegen: temp_%d found as result with type %d in instruction %zu (opcode: %s)\n",
                           i, temp_type, j, ir_opcode_to_string(instr->opcode));
                }
            }

            if (!found_as_result)
            {
                if (instr->arg1 && instr->arg1->type == IR_OP_TEMP &&
                    instr->arg1->data.temp_id == i)
                {
                    temp_type = instr->arg1->data_type;
                    if (debug_enabled)
                    {
                        printf("[DEBUG] codegen: temp_%d found as arg1 with type %d in instruction %zu (opcode: %s)\n",
                               i, temp_type, j, ir_opcode_to_string(instr->opcode));
                    }
                }

                if (instr->arg2 && instr->arg2->type == IR_OP_TEMP &&
                    instr->arg2->data.temp_id == i)
                {
                    temp_type = instr->arg2->data_type;
                    if (debug_enabled)
                    {
                        printf("[DEBUG] codegen: temp_%d found as arg2 with type %d in instruction %zu (opcode: %s)\n",
                               i, temp_type, j, ir_opcode_to_string(instr->opcode));
                    }
                }
            }
        }

        const char *c_type = codegen_c_writer_get_c_type_string(temp_type);
        if (debug_enabled)
        {
            printf("[DEBUG] codegen: temp_%d final type: %s\n", i, c_type);
        }
        codegen_c_writer_write_line(generator, "%s %s;", c_type, temp_name);
    }

    if (func->temp_counter > 0)
    {
        fprintf(generator->output_file, "\n");
    }
}

void codegen_c_writer_write_function_footer(CodeGenerator *generator)
{
    generator->indent_level--;
    fprintf(generator->output_file, "}\n\n");
}

void codegen_c_writer_write_main_function(CodeGenerator *generator)
{
    fprintf(generator->output_file, "int main() {\n");
    fprintf(generator->output_file, "    return 0;\n");
    fprintf(generator->output_file, "}\n");
}

void codegen_c_writer_write_operand(CodeGenerator *generator, IROperand *operand)
{
    if (!operand)
    {
        fprintf(generator->output_file, "NULL");
        return;
    }

    switch (operand->type)
    {
    case IR_OP_CONST:
        if (operand->data_type == TYPE_FLOAT || operand->data_type == TYPE_DOUBLE || operand->is_float_const)
        {
            fprintf(generator->output_file, "%f", operand->data.float_const_value);
        }
        else if (operand->data_type == TYPE_BOOL)
        {
            fprintf(generator->output_file, "%s", operand->data.const_value ? "true" : "false");
        }
        else
        {
            fprintf(generator->output_file, "%lld", operand->data.const_value);
        }
        break;

    case IR_OP_STRING_CONST:
        fprintf(generator->output_file, "\"");
        escape_string_for_c(operand->data.string_const_value, generator->output_file);
        fprintf(generator->output_file, "\"");
        break;

    case IR_OP_VAR:
        fprintf(generator->output_file, "%s", operand->data.var_name);
        break;

    case IR_OP_TEMP:
        fprintf(generator->output_file, "temp_%d", operand->data.temp_id);
        break;

    case IR_OP_NULL:
        fprintf(generator->output_file, "NULL");
        break;

    default:
        fprintf(generator->output_file, "UNKNOWN");
        break;
    }
}

void codegen_c_writer_write_indent(CodeGenerator *generator)
{
    for (int i = 0; i < generator->indent_level; i++)
    {
        fprintf(generator->output_file, "    ");
    }
}

void codegen_c_writer_write_line(CodeGenerator *generator, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    codegen_c_writer_write_indent(generator);
    vfprintf(generator->output_file, format, args);
    fprintf(generator->output_file, "\n");

    va_end(args);
}

const char *codegen_c_writer_get_c_type_string(DataType type)
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

const char *get_printf_format(DataType type)
{
    switch (type)
    {
    case TYPE_STRING:
        return "%s";
    case TYPE_FLOAT:
    case TYPE_DOUBLE:
        return "%f";
    case TYPE_BOOL:
        return "%d";
    case TYPE_INT:
    default:
        return "%lld";
    }
}

bool is_float_type(DataType type)
{
    return type == TYPE_FLOAT || type == TYPE_DOUBLE;
}
