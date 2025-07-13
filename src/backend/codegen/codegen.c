#include "backend/codegen.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *get_c_type_string(DataType type);

CodeGenerator *codegen_create(IRProgram *ir_program, FILE *output_file, Error *error)
{
    CodeGenerator *generator = safe_malloc(sizeof(CodeGenerator));
    generator->ir_program = ir_program;
    generator->output_file = output_file;
    generator->error = error;
    generator->indent_level = 0;
    generator->temp_counter = 0;
    generator->temp_map = hashtable_create(16);
    generator->var_set = hashtable_create(16);
    generator->array_info = hashtable_create(16);
    generator->variable_types = hashtable_create(16);
    generator->param_count = 0;
    return generator;
}

void codegen_destroy(CodeGenerator *generator)
{
    if (!generator)
        return;

    hashtable_destroy(generator->temp_map);
    hashtable_destroy(generator->var_set);
    hashtable_destroy(generator->array_info);
    hashtable_destroy(generator->variable_types);
    safe_free(generator);
}

bool codegen_generate(CodeGenerator *generator)
{
    codegen_write_header(generator);
    codegen_generate_program(generator);
    return true;
}

void codegen_generate_program(CodeGenerator *generator)
{
    for (size_t i = 0; i < generator->ir_program->functions.size; i++)
    {
        IRFunction *func = (IRFunction *)array_get(&generator->ir_program->functions, i);
        codegen_generate_function(generator, func);
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
        codegen_write_main_function(generator);
    }
}

void codegen_generate_function(CodeGenerator *generator, IRFunction *func)
{
    codegen_write_function_header(generator, func);

    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (instr->opcode == IR_ARRAY_DECL || instr->opcode == IR_VAR_DECL)
            continue;
        codegen_generate_instruction(generator, instr);
    }

    codegen_write_function_footer(generator);
}

void codegen_generate_instruction(CodeGenerator *generator, IRInstruction *instr)
{
    if (!instr)
        return;

    switch (instr->opcode)
    {
    case IR_NOP:
        break;

    case IR_LABEL:
        codegen_write_line(generator, "%s:", instr->label);
        break;

    case IR_MOVE:
        codegen_write_indent(generator);
        codegen_write_operand(generator, instr->result);
        fprintf(generator->output_file, " = ");
        if (instr->arg1 && instr->arg1->type == IR_OP_NULL)
        {
            if (instr->result && instr->result->data_type == TYPE_INT)
            {
                fprintf(generator->output_file, "0");
            }
            else if (instr->result && instr->result->data_type == TYPE_BOOL)
            {
                fprintf(generator->output_file, "false");
            }
            else if (instr->result && instr->result->data_type == TYPE_FLOAT)
            {
                fprintf(generator->output_file, "0.0f");
            }
            else if (instr->result && instr->result->data_type == TYPE_DOUBLE)
            {
                fprintf(generator->output_file, "0.0");
            }
            else if (instr->result && instr->result->data_type == TYPE_STRING)
            {
                fprintf(generator->output_file, "NULL");
            }
            else
            {
                fprintf(generator->output_file, "NULL");
            }
        }
        else
        {
            codegen_write_operand(generator, instr->arg1);
        }
        fprintf(generator->output_file, ";\n");
        break;

    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
    case IR_MOD:
        codegen_write_indent(generator);
        codegen_write_operand(generator, instr->result);
        fprintf(generator->output_file, " = ");

        bool use_float = false;
        if (instr->arg1 && (instr->arg1->data_type == TYPE_FLOAT || instr->arg1->data_type == TYPE_DOUBLE))
        {
            use_float = true;
        }
        if (instr->arg2 && (instr->arg2->data_type == TYPE_FLOAT || instr->arg2->data_type == TYPE_DOUBLE))
        {
            use_float = true;
        }

        if (use_float)
        {
            codegen_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, " %s ", ir_opcode_to_string(instr->opcode));
            codegen_write_operand(generator, instr->arg2);
        }
        else
        {
            codegen_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, " %s ", ir_opcode_to_string(instr->opcode));
            codegen_write_operand(generator, instr->arg2);
        }
        fprintf(generator->output_file, ";\n");
        break;

    case IR_NEG:
    case IR_NOT:
        codegen_write_indent(generator);
        codegen_write_operand(generator, instr->result);
        fprintf(generator->output_file, " = %s",
                instr->opcode == IR_NEG ? "-" : "!");
        codegen_write_operand(generator, instr->arg1);
        fprintf(generator->output_file, ";\n");
        break;

    case IR_JUMP:
        codegen_write_indent(generator);
        fprintf(generator->output_file, "goto %s;\n", instr->label);
        break;

    case IR_JUMP_IF:
        codegen_write_indent(generator);
        fprintf(generator->output_file, "if (");
        codegen_write_operand(generator, instr->arg1);
        fprintf(generator->output_file, ") goto %s;\n", instr->label);
        break;

    case IR_JUMP_IF_FALSE:
        codegen_write_indent(generator);
        fprintf(generator->output_file, "if (!");
        codegen_write_operand(generator, instr->arg1);
        fprintf(generator->output_file, ") goto %s;\n", instr->label);
        break;

    case IR_PARAM:
        if (generator->param_count < MAX_PARAMS)
        {
            generator->params[generator->param_count++] = instr->arg1;
        }
        break;

    case IR_CALL:
        codegen_write_indent(generator);
        if (instr->result)
        {
            codegen_write_operand(generator, instr->result);
            fprintf(generator->output_file, " = ");
        }

        const char *func_name = instr->label;
        if (strcmp(func_name, "concat") == 0 ||
            strcmp(func_name, "substr") == 0 ||
            strcmp(func_name, "strlen") == 0 ||
            strcmp(func_name, "strcmp") == 0 ||
            strcmp(func_name, "char_at") == 0)
        {
            fprintf(generator->output_file, "__tl_%s(", func_name);
        }
        else
        {
            fprintf(generator->output_file, "%s(", func_name);
        }

        for (int i = 0; i < generator->param_count; i++)
        {
            if (i > 0)
                fprintf(generator->output_file, ", ");
            codegen_write_operand(generator, generator->params[i]);
        }

        fprintf(generator->output_file, ");\n");
        generator->param_count = 0;
        break;

    case IR_RETURN:
        codegen_write_indent(generator);
        if (instr->arg1)
        {
            fprintf(generator->output_file, "return ");
            codegen_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, ";\n");
        }
        else
        {
            fprintf(generator->output_file, "return 0;\n");
        }
        break;

    case IR_PRINT:
        codegen_write_indent(generator);
        if (instr->arg1)
        {
            if (instr->arg1->data_type == TYPE_STRING)
            {
                fprintf(generator->output_file, "printf(\"%%s\\n\", ");
                codegen_write_operand(generator, instr->arg1);
                fprintf(generator->output_file, ");\n");
            }
            else if (instr->arg1->data_type == TYPE_FLOAT || instr->arg1->data_type == TYPE_DOUBLE || instr->arg1->is_float_const)
            {
                fprintf(generator->output_file, "printf(\"%%f\\n\", ");
                codegen_write_operand(generator, instr->arg1);
                fprintf(generator->output_file, ");\n");
            }
            else if (instr->arg1->data_type == TYPE_BOOL)
            {
                fprintf(generator->output_file, "printf(\"%%d\\n\", ");
                codegen_write_operand(generator, instr->arg1);
                fprintf(generator->output_file, ");\n");
            }
            else
            {
                fprintf(generator->output_file, "printf(\"%%lld\\n\", ");
                codegen_write_operand(generator, instr->arg1);
                fprintf(generator->output_file, ");\n");
            }
        }
        break;
    case IR_ARRAY_LOAD:
        codegen_write_indent(generator);
        codegen_write_operand(generator, instr->result);
        fprintf(generator->output_file, " = ");
        codegen_write_operand(generator, instr->arg1);
        fprintf(generator->output_file, "[");
        codegen_write_operand(generator, instr->arg2);
        fprintf(generator->output_file, "];\n");
        break;
    case IR_ARRAY_STORE:
        codegen_write_indent(generator);
        codegen_write_operand(generator, instr->arg1);
        fprintf(generator->output_file, "[");
        codegen_write_operand(generator, instr->arg2);
        fprintf(generator->output_file, "] = ");
        codegen_write_operand(generator, instr->result);
        fprintf(generator->output_file, ";\n");
        break;
    case IR_BOUNDS_CHECK:
        codegen_write_indent(generator);
        fprintf(generator->output_file, "if (");
        codegen_write_operand(generator, instr->arg1);
        fprintf(generator->output_file, " >= ");
        codegen_write_operand(generator, instr->arg2);
        fprintf(generator->output_file, ") {\n");
        generator->indent_level++;
        codegen_write_indent(generator);
        fprintf(generator->output_file, "fprintf(stderr, \"Array index out of bounds\\n\");\n");
        codegen_write_indent(generator);
        fprintf(generator->output_file, "exit(1);\n");
        generator->indent_level--;
        codegen_write_indent(generator);
        fprintf(generator->output_file, "}\n");
        break;
    case IR_ARRAY_DECL:
        break;
    case IR_ARRAY_INIT:
        codegen_write_indent(generator);
        fprintf(generator->output_file, "for (int i = 0; i < %d; i++) {\n", instr->result->array_size);
        generator->indent_level++;
        codegen_write_indent(generator);
        codegen_write_operand(generator, instr->result);
        fprintf(generator->output_file, "[i] = ");
        codegen_write_operand(generator, instr->arg1);
        fprintf(generator->output_file, ";\n");
        generator->indent_level--;
        codegen_write_indent(generator);
        fprintf(generator->output_file, "}\n");
        break;
    case IR_VAR_DECL:
        break;
    case IR_EQ:
    case IR_NE:
    case IR_LT:
    case IR_LE:
    case IR_GT:
    case IR_GE:
    case IR_AND:
    case IR_OR:
        codegen_write_indent(generator);
        codegen_write_operand(generator, instr->result);
        fprintf(generator->output_file, " = ");

        codegen_write_operand(generator, instr->arg1);
        fprintf(generator->output_file, " %s ", ir_opcode_to_string(instr->opcode));
        codegen_write_operand(generator, instr->arg2);
        fprintf(generator->output_file, ";\n");
        break;
    }
}

void codegen_write_indent(CodeGenerator *generator)
{
    for (int i = 0; i < generator->indent_level; i++)
    {
        fprintf(generator->output_file, "    ");
    }
}

void codegen_write_line(CodeGenerator *generator, const char *format, ...)
{
    va_list args;
    va_start(args, format);

    codegen_write_indent(generator);
    vfprintf(generator->output_file, format, args);
    fprintf(generator->output_file, "\n");

    va_end(args);
}

void codegen_write_operand(CodeGenerator *generator, IROperand *operand)
{
    if (!operand)
    {
        fprintf(generator->output_file, "0");
        return;
    }

    switch (operand->type)
    {
    case IR_OP_TEMP:
    {
        char temp_name[32];
        snprintf(temp_name, sizeof(temp_name), "temp_%d", operand->data.temp_id);
        fprintf(generator->output_file, "%s", temp_name);
        break;
    }
    case IR_OP_VAR:
        fprintf(generator->output_file, "%s", operand->data.var_name);
        break;
    case IR_OP_CONST:
        if (operand->is_float_const)
        {
            double float_value = (double)operand->data.const_value / 1000000.0;
            fprintf(generator->output_file, "%f", float_value);
        }
        else
        {
            fprintf(generator->output_file, "%lld", operand->data.const_value);
        }
        break;
    case IR_OP_STRING_CONST:
        fprintf(generator->output_file, "\"%s\"", operand->data.string_const_value);
        break;
    case IR_OP_LABEL:
        fprintf(generator->output_file, "%s", operand->data.label_name);
        break;
    case IR_OP_NULL:
        if (operand->data_type == TYPE_INT)
        {
            fprintf(generator->output_file, "0");
        }
        else if (operand->data_type == TYPE_BOOL)
        {
            fprintf(generator->output_file, "false");
        }
        else if (operand->data_type == TYPE_STRING)
        {
            fprintf(generator->output_file, "NULL");
        }
        else
        {
            fprintf(generator->output_file, "NULL");
        }
        break;
    }
}

char *codegen_get_temp_name(CodeGenerator *generator, IROperand *operand)
{
    if (operand->type != IR_OP_TEMP)
        return NULL;

    char temp_key[32];
    snprintf(temp_key, sizeof(temp_key), "t%d", operand->data.temp_id);

    char *existing_name = hashtable_get(generator->temp_map, temp_key);
    if (existing_name)
    {
        return existing_name;
    }

    char *new_name = safe_malloc(32);
    snprintf(new_name, 32, "temp_%d", generator->temp_counter++);
    hashtable_put(generator->temp_map, temp_key, new_name);

    return new_name;
}

void codegen_write_header(CodeGenerator *generator)
{
    fprintf(generator->output_file, "#include <stdio.h>\n");
    fprintf(generator->output_file, "#include <stdlib.h>\n");
    fprintf(generator->output_file, "#include <stdint.h>\n");
    fprintf(generator->output_file, "#include <stdbool.h>\n");
    fprintf(generator->output_file, "#include <inttypes.h>\n");
    fprintf(generator->output_file, "#include <string.h>\n");
    fprintf(generator->output_file, "\n");
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
    fprintf(generator->output_file, "    return (int64_t)strcmp(a, b);\n");
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

    for (size_t i = 0; i < generator->ir_program->functions.size; i++)
    {
        IRFunction *func = (IRFunction *)array_get(&generator->ir_program->functions, i);
        if (!string_equal(func->name, "main"))
        {
            DataType return_type = func->return_type;

            const char *return_type_str = get_c_type_string(return_type);
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
                    const char *param_type = get_c_type_string(param->data_type);
                    fprintf(generator->output_file, "%s %s", param_type, param->data.var_name);
                }
            }

            fprintf(generator->output_file, ");\n");
        }
    }
    fprintf(generator->output_file, "\n");
}

const char *get_c_type_string(DataType type)
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

bool codegen_is_array_variable(CodeGenerator *generator, const char *var_name)
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

int codegen_get_array_size(CodeGenerator *generator, const char *var_name)
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

void codegen_write_function_header(CodeGenerator *generator, IRFunction *func)
{
    if (string_equal(func->name, "main"))
    {
        fprintf(generator->output_file, "int main(void) {\n");
    }
    else
    {
        DataType return_type = func->return_type;

        const char *return_type_str = get_c_type_string(return_type);
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
                const char *param_type = get_c_type_string(param->data_type);
                fprintf(generator->output_file, "%s %s", param_type, param->data.var_name);
            }
        }

        fprintf(generator->output_file, ") {\n");
    }

    generator->indent_level++;

    for (size_t j = 0; j < func->instructions.size; j++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, j);
        if (instr->opcode == IR_ARRAY_DECL)
        {
            if (instr->result && instr->result->type == IR_OP_VAR)
            {
                codegen_write_indent(generator);
                const char *c_type = get_c_type_string(instr->result->data_type);
                fprintf(generator->output_file, "%s %s[%d];\n", c_type, instr->result->data.var_name, instr->result->array_size);
            }
        }
        else if (instr->opcode == IR_VAR_DECL)
        {
            if (instr->result && instr->result->type == IR_OP_VAR)
            {
                codegen_write_indent(generator);
                const char *c_type = get_c_type_string(instr->result->data_type);
                fprintf(generator->output_file, "%s %s;\n", c_type, instr->result->data.var_name);
            }
        }
    }
    for (int i = 0; i < func->temp_counter; i++)
    {
        char temp_name[32];
        snprintf(temp_name, sizeof(temp_name), "temp_%d", i);
        DataType temp_type = TYPE_INT;
        for (size_t j = 0; j < func->instructions.size; j++)
        {
            IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, j);
            if (instr->result && instr->result->type == IR_OP_TEMP &&
                instr->result->data.temp_id == i)
            {
                temp_type = instr->result->data_type;
                break;
            }
            if (instr->opcode == IR_ARRAY_LOAD)
            {
                if (instr->result && instr->result->type == IR_OP_TEMP && instr->result->data.temp_id == i)
                {
                    temp_type = instr->result->data_type;
                    break;
                }
            }
        }
        const char *c_type = get_c_type_string(temp_type);
        codegen_write_line(generator, "%s %s;", c_type, temp_name);
    }

    if (func->temp_counter > 0)
    {
        fprintf(generator->output_file, "\n");
    }
}

void codegen_write_function_footer(CodeGenerator *generator)
{
    generator->indent_level--;
    fprintf(generator->output_file, "}\n\n");
}

void codegen_write_main_function(CodeGenerator *generator)
{
    fprintf(generator->output_file, "int main() {\n");
    fprintf(generator->output_file, "    return 0;\n");
    fprintf(generator->output_file, "}\n");
}

void codegen_error(CodeGenerator *generator, const char *message)
{
    if (generator->error)
    {
        error_set(generator->error, ERROR_CODEGEN, message, 0, 0);
    }
}