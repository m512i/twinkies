#include "backend/codegen_instruction_handlers.h"
#include "backend/codegen_c_writer.h"
#include "backend/codegen_ffi.h"
#include "flags.h"
#include <stdio.h>
#include <string.h>

extern bool debug_enabled;

void codegen_instruction_handlers_generate_instruction(CodeGenerator *generator, IRInstruction *instr)
{
    if (!instr)
        return;

    switch (instr->opcode)
    {
    case IR_NOP:
        codegen_handle_nop(generator, instr);
        break;
    case IR_LABEL:
        codegen_handle_label(generator, instr);
        break;
    case IR_MOVE:
        codegen_handle_move(generator, instr);
        break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
    case IR_MOD:
        codegen_handle_arithmetic(generator, instr);
        break;
    case IR_NEG:
    case IR_NOT:
        codegen_handle_unary_arithmetic(generator, instr);
        break;
    case IR_JUMP:
        codegen_handle_jump(generator, instr);
        break;
    case IR_JUMP_IF:
        codegen_handle_jump_if(generator, instr);
        break;
    case IR_JUMP_IF_FALSE:
        codegen_handle_jump_if_false(generator, instr);
        break;
    case IR_PARAM:
        codegen_handle_param(generator, instr);
        break;
    case IR_CALL:
        codegen_handle_call(generator, instr);
        break;
    case IR_RETURN:
        codegen_handle_return(generator, instr);
        break;
    case IR_PRINT:
        codegen_handle_print(generator, instr);
        break;
    case IR_ARRAY_LOAD:
        codegen_handle_array_load(generator, instr);
        break;
    case IR_ARRAY_STORE:
        codegen_handle_array_store(generator, instr);
        break;
    case IR_BOUNDS_CHECK:
        codegen_handle_bounds_check(generator, instr);
        break;
    case IR_ARRAY_DECL:
        codegen_handle_array_decl(generator, instr);
        break;
    case IR_ARRAY_INIT:
        codegen_handle_array_init(generator, instr);
        break;
    case IR_VAR_DECL:
        codegen_handle_var_decl(generator, instr);
        break;
    case IR_EQ:
    case IR_NE:
    case IR_LT:
    case IR_LE:
    case IR_GT:
    case IR_GE:
    case IR_AND:
    case IR_OR:
        codegen_handle_comparison(generator, instr);
        break;
    }
}

void codegen_handle_nop(CodeGenerator *generator, IRInstruction *instr)
{
    (void)generator;
    (void)instr;
}

void codegen_handle_label(CodeGenerator *generator, IRInstruction *instr)
{
    codegen_core_write_line(generator, "%s:", instr->label);
}

void codegen_handle_jump(CodeGenerator *generator, IRInstruction *instr)
{
    codegen_core_write_indent(generator);
    fprintf(generator->output_file, "goto %s;\n", instr->label);
}

void codegen_handle_jump_if(CodeGenerator *generator, IRInstruction *instr)
{
    codegen_core_write_indent(generator);
    fprintf(generator->output_file, "if (");
    codegen_c_writer_write_operand(generator, instr->arg1);
    fprintf(generator->output_file, ") goto %s;\n", instr->label);
}

void codegen_handle_jump_if_false(CodeGenerator *generator, IRInstruction *instr)
{
    codegen_core_write_indent(generator);
    fprintf(generator->output_file, "if (!");
    codegen_c_writer_write_operand(generator, instr->arg1);
    fprintf(generator->output_file, ") goto %s;\n", instr->label);
}

void codegen_handle_return(CodeGenerator *generator, IRInstruction *instr)
{
    codegen_core_write_indent(generator);
    if (instr->arg1)
    {
        fprintf(generator->output_file, "return ");
        codegen_c_writer_write_operand(generator, instr->arg1);
        fprintf(generator->output_file, ";\n");
    }
    else
    {
        fprintf(generator->output_file, "return 0;\n");
    }
}

void codegen_handle_move(CodeGenerator *generator, IRInstruction *instr)
{
    codegen_core_write_indent(generator);
    codegen_c_writer_write_operand(generator, instr->result);
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
        codegen_c_writer_write_operand(generator, instr->arg1);
    }
    fprintf(generator->output_file, ";\n");
}

void codegen_handle_arithmetic(CodeGenerator *generator, IRInstruction *instr)
{
    codegen_core_write_indent(generator);
    codegen_c_writer_write_operand(generator, instr->result);
    fprintf(generator->output_file, " = ");

    codegen_c_writer_write_operand(generator, instr->arg1);
    fprintf(generator->output_file, " %s ", ir_opcode_to_string(instr->opcode));
    codegen_c_writer_write_operand(generator, instr->arg2);
    fprintf(generator->output_file, ";\n");
}

void codegen_handle_unary_arithmetic(CodeGenerator *generator, IRInstruction *instr)
{
    codegen_core_write_indent(generator);
    codegen_c_writer_write_operand(generator, instr->result);
    fprintf(generator->output_file, " = %s",
            instr->opcode == IR_NEG ? "-" : "!");
    codegen_c_writer_write_operand(generator, instr->arg1);
    fprintf(generator->output_file, ";\n");
}

void codegen_handle_comparison(CodeGenerator *generator, IRInstruction *instr)
{
    codegen_core_write_indent(generator);
    codegen_c_writer_write_operand(generator, instr->result);
    fprintf(generator->output_file, " = ");

    codegen_c_writer_write_operand(generator, instr->arg1);
    fprintf(generator->output_file, " %s ", ir_opcode_to_string(instr->opcode));
    codegen_c_writer_write_operand(generator, instr->arg2);
    fprintf(generator->output_file, ";\n");
}

void codegen_handle_param(CodeGenerator *generator, IRInstruction *instr)
{
    if (generator->param_count < MAX_PARAMS)
    {
        generator->params[generator->param_count++] = instr->arg1;
    }
}

void codegen_handle_call(CodeGenerator *generator, IRInstruction *instr)
{
    codegen_core_write_indent(generator);
    if (instr->result)
    {
        codegen_c_writer_write_operand(generator, instr->result);
        fprintf(generator->output_file, " = ");
    }

    const char *func_name = instr->label;
    
    if (codegen_is_ffi_function(generator, func_name))
    {
        fprintf(generator->output_file, "ffi_%s(", func_name);
    }
    else if (strcmp(func_name, "concat") == 0 ||
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
        codegen_c_writer_write_operand(generator, generator->params[i]);
    }

    fprintf(generator->output_file, ");\n");
    generator->param_count = 0;
}

void codegen_handle_print(CodeGenerator *generator, IRInstruction *instr)
{
    codegen_core_write_indent(generator);
    if (instr->args)
    {
        fprintf(generator->output_file, "printf(\"");
        for (size_t i = 0; i < instr->args->size; i++)
        {
            IROperand *arg = (IROperand *)array_get(instr->args, i);
            if (arg->data_type == TYPE_STRING)
            {
                fprintf(generator->output_file, "%%s");
            }
            else if (arg->data_type == TYPE_FLOAT || arg->data_type == TYPE_DOUBLE || arg->is_float_const)
            {
                fprintf(generator->output_file, "%%f");
            }
            else if (arg->data_type == TYPE_BOOL)
            {
                fprintf(generator->output_file, "%%d");
            }
            else
            {
                fprintf(generator->output_file, "%%lld");
            }
        }
        fprintf(generator->output_file, "\\n\"");
        
        for (size_t i = 0; i < instr->args->size; i++)
        {
            fprintf(generator->output_file, ", ");
            codegen_c_writer_write_operand(generator, (IROperand *)array_get(instr->args, i));
        }
        fprintf(generator->output_file, ");\n");
    }
    else if (instr->arg1)
    {
        if (instr->arg1->data_type == TYPE_STRING)
        {
            fprintf(generator->output_file, "printf(\"%%s\\n\", ");
            codegen_c_writer_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, ");\n");
        }
        else if (instr->arg1->data_type == TYPE_FLOAT || instr->arg1->data_type == TYPE_DOUBLE || instr->arg1->is_float_const)
        {
            fprintf(generator->output_file, "printf(\"%%f\\n\", ");
            codegen_c_writer_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, ");\n");
        }
        else if (instr->arg1->data_type == TYPE_BOOL)
        {
            fprintf(generator->output_file, "printf(\"%%d\\n\", ");
            codegen_c_writer_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, ");\n");
        }
        else
        {
            fprintf(generator->output_file, "printf(\"%%lld\\n\", ");
            codegen_c_writer_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, ");\n");
        }
    }
}

void codegen_handle_array_load(CodeGenerator *generator, IRInstruction *instr)
{
    codegen_core_write_indent(generator);
    codegen_c_writer_write_operand(generator, instr->result);
    fprintf(generator->output_file, " = ");
    codegen_c_writer_write_operand(generator, instr->arg1);
    fprintf(generator->output_file, "[");
    codegen_c_writer_write_operand(generator, instr->arg2);
    fprintf(generator->output_file, "];\n");
}

void codegen_handle_array_store(CodeGenerator *generator, IRInstruction *instr)
{
    codegen_core_write_indent(generator);
    codegen_c_writer_write_operand(generator, instr->arg1);
    fprintf(generator->output_file, "[");
    codegen_c_writer_write_operand(generator, instr->arg2);
    fprintf(generator->output_file, "] = ");
    codegen_c_writer_write_operand(generator, instr->result);
    fprintf(generator->output_file, ";\n");
}

void codegen_handle_bounds_check(CodeGenerator *generator, IRInstruction *instr)
{
    codegen_core_write_indent(generator);
    fprintf(generator->output_file, "if (");
    codegen_c_writer_write_operand(generator, instr->arg1);
    fprintf(generator->output_file, " >= ");
    codegen_c_writer_write_operand(generator, instr->arg2);
    fprintf(generator->output_file, ") {\n");
    generator->indent_level++;
    codegen_core_write_indent(generator);
    fprintf(generator->output_file, "fprintf(stderr, \"Array index out of bounds\\n\");\n");
    codegen_core_write_indent(generator);
    fprintf(generator->output_file, "exit(1);\n");
    generator->indent_level--;
    codegen_core_write_indent(generator);
    fprintf(generator->output_file, "}\n");
}

void codegen_handle_array_decl(CodeGenerator *generator, IRInstruction *instr)
{
    (void)generator;
    (void)instr;
    // Array declarations are handled in function header generation
}

void codegen_handle_array_init(CodeGenerator *generator, IRInstruction *instr)
{
    codegen_core_write_indent(generator);
    fprintf(generator->output_file, "for (int i = 0; i < %d; i++) {\n", instr->result->array_size);
    generator->indent_level++;
    codegen_core_write_indent(generator);
    codegen_c_writer_write_operand(generator, instr->result);
    fprintf(generator->output_file, "[i] = ");
    codegen_c_writer_write_operand(generator, instr->arg1);
    fprintf(generator->output_file, ";\n");
    generator->indent_level--;
    codegen_core_write_indent(generator);
    fprintf(generator->output_file, "}\n");
}

void codegen_handle_var_decl(CodeGenerator *generator, IRInstruction *instr)
{
    (void)generator;
    (void)instr;
    // Variable declarations are handled in function header generation
}
