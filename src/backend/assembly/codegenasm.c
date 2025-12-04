#include "backend/codegen/codegen.h"
#include "backend/ir/irOps.h"
#include <stdarg.h>
extern bool debug_enabled;

void codegenasm_write_text_section(CodeGenerator *generator);
void codegenasm_write_data_section(CodeGenerator *generator);

static bool is_param_register(const char *name)
{
    return strcmp(name, "rcx") == 0 || strcmp(name, "rdx") == 0 || strcmp(name, "r8") == 0 || strcmp(name, "r9") == 0;
}

void codegenasm_call_import(CodeGenerator *generator, const char *func_name, IROperand *value)
{
    fprintf(generator->output_file, "    lea rcx, [rel format_int]\n");
    fprintf(generator->output_file, "    mov rdx, qword [rel %s]\n",
            codegenasm_get_operand_name(generator, value));
    fprintf(generator->output_file, "    mov rax, qword [rel __imp_%s]\n", func_name);
    fprintf(generator->output_file, "    call rax\n");
}

typedef struct
{
    IROperand *operand;
    const char *register_name;
    bool is_live;
} RegisterAllocation;

#define MAX_REGISTERS 6
// static const char* available_registers[] = {"rax", "rcx", "rdx", "r8", "r9", "r10"};

// Remove unused function or add proper implementation
/*
RegisterAllocation* find_register_for_operand(CodeGenerator* generator, IROperand* operand) {
    // For now, we'll use a simple approach - just use rax for most operations
    // In a full implementation, you'd track register usage and allocate optimally - fix this alan
    static RegisterAllocation rax_allocation = {NULL, "rax", false};
    rax_allocation.operand = operand;
    rax_allocation.is_live = true;
    return &rax_allocation;
}
*/

CodeGenerator *codegenasm_create(IRProgram *ir_program, FILE *output_file, Error *error)
{
    CodeGenerator *generator = safe_malloc(sizeof(CodeGenerator));
    generator->ir_program = ir_program;
    generator->output_file = output_file;
    generator->error = error;
    generator->indent_level = 0;
    generator->temp_counter = 0;
    generator->temp_map = hashtable_create(16);
    generator->var_set = hashtable_create(16);
    generator->declared_temps = hashtable_create(16);
    generator->param_count = 0;
    generator->current_function_name = NULL;
    return generator;
}

void codegenasm_destroy(CodeGenerator *generator)
{
    if (!generator)
        return;

    hashtable_destroy(generator->temp_map);
    hashtable_destroy(generator->var_set);
    hashtable_destroy(generator->declared_temps);
    safe_free(generator);
}

bool codegenasm_generate(CodeGenerator *generator)
{
    if (debug_enabled)
    {
        printf("[DEBUG] Entered codegenasm_generate\n");
        fflush(stdout);
    }
    codegenasm_write_header(generator);
    if (debug_enabled)
    {
        printf("[DEBUG] Wrote header\n");
        fflush(stdout);
    }
    codegenasm_generate_program(generator);
    if (debug_enabled)
    {
        printf("[DEBUG] Generated program\n");
        fflush(stdout);
    }
    return true;
}

void codegenasm_generate_program(CodeGenerator *generator)
{
    if (debug_enabled)
    {
        printf("[DEBUG] Entered codegenasm_generate_program\n");
        fflush(stdout);
    }
    if (debug_enabled)
    {
        printf("[DEBUG] Number of functions: %zu\n", generator->ir_program->functions.size);
        fflush(stdout);
    }

    codegenasm_write_data_section(generator);
    if (debug_enabled)
    {
        printf("[DEBUG] Wrote data section\n");
        fflush(stdout);
    }

    codegenasm_write_text_section(generator);
    if (debug_enabled)
    {
        printf("[DEBUG] Wrote text section\n");
        fflush(stdout);
    }

    for (size_t i = 0; i < generator->ir_program->functions.size; i++)
    {
        IRFunction *func = (IRFunction *)array_get(&generator->ir_program->functions, i);
        if (debug_enabled)
        {
            printf("[DEBUG] Generating function: %s\n", func->name);
            fflush(stdout);
        }
        codegenasm_generate_function(generator, func);
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
            printf("[DEBUG] Adding main function\n");
            fflush(stdout);
        }
        codegenasm_write_main_function(generator);
    }

    if (debug_enabled)
    {
        printf("[DEBUG] Exiting codegenasm_generate_program\n");
        fflush(stdout);
    }
}

void codegenasm_generate_function(CodeGenerator *generator, IRFunction *func)
{
    if (debug_enabled)
    {
        printf("[DEBUG] Entered codegenasm_generate_function for %s\n", func->name);
        fflush(stdout);
    }
    if (debug_enabled)
    {
        printf("[DEBUG] Number of instructions: %zu\n", func->instructions.size);
        fflush(stdout);
    }

    generator->current_function_name = func->name;
    snprintf(generator->epilogue_label, sizeof(generator->epilogue_label), "%s_epilogue", func->name);

    codegenasm_write_function_header(generator, func);

    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (debug_enabled)
        {
            printf("[DEBUG] Generating instruction %zu: %s\n", i, ir_opcode_to_string(instr->opcode));
            fflush(stdout);
        }
        codegenasm_generate_instruction(generator, instr);
    }

    fprintf(generator->output_file, "%s:\n", generator->epilogue_label);
    codegenasm_write_function_footer(generator);
    if (debug_enabled)
    {
        printf("[DEBUG] Exiting codegenasm_generate_function for %s\n", func->name);
        fflush(stdout);
    }
}

void codegenasm_generate_instruction(CodeGenerator *generator, IRInstruction *instr)
{
    if (!instr)
        return;

    switch (instr->opcode)
    {
    case IR_NOP:
        fprintf(generator->output_file, "    nop\n");
        break;

    case IR_LABEL:
        fprintf(generator->output_file, "%s_%s:\n", generator->current_function_name, instr->label);
        break;

    case IR_MOVE:
        codegenasm_move(generator, instr->result, instr->arg1);
        break;

    case IR_ADD:
        codegenasm_binary_op(generator, "add", instr->result, instr->arg1, instr->arg2);
        break;

    case IR_SUB:
        codegenasm_binary_op(generator, "sub", instr->result, instr->arg1, instr->arg2);
        break;

    case IR_MUL:
        codegenasm_mul(generator, instr->result, instr->arg1, instr->arg2);
        break;

    case IR_DIV:
        codegenasm_div(generator, instr->result, instr->arg1, instr->arg2);
        break;

    case IR_MOD:
        codegenasm_mod(generator, instr->result, instr->arg1, instr->arg2);
        break;

    case IR_NEG:
        codegenasm_unary_op(generator, "neg", instr->result, instr->arg1);
        break;

    case IR_NOT:
        codegenasm_not(generator, instr->result, instr->arg1);
        break;

    case IR_EQ:
        codegenasm_compare(generator, "sete", instr->result, instr->arg1, instr->arg2);
        break;

    case IR_NE:
        codegenasm_compare(generator, "setne", instr->result, instr->arg1, instr->arg2);
        break;

    case IR_LT:
        codegenasm_compare(generator, "setl", instr->result, instr->arg1, instr->arg2);
        break;

    case IR_LE:
        codegenasm_compare(generator, "setle", instr->result, instr->arg1, instr->arg2);
        break;

    case IR_GT:
        codegenasm_compare(generator, "setg", instr->result, instr->arg1, instr->arg2);
        break;

    case IR_GE:
        codegenasm_compare(generator, "setge", instr->result, instr->arg1, instr->arg2);
        break;

    case IR_AND:
        codegenasm_binary_op(generator, "and", instr->result, instr->arg1, instr->arg2);
        break;

    case IR_OR:
        codegenasm_binary_op(generator, "or", instr->result, instr->arg1, instr->arg2);
        break;

    case IR_JUMP:
        fprintf(generator->output_file, "    jmp %s_%s\n", generator->current_function_name, instr->label);
        break;

    case IR_JUMP_IF:
        fprintf(generator->output_file, "    cmp qword [rel %s], 0\n",
                codegenasm_get_operand_name(generator, instr->arg1));
        fprintf(generator->output_file, "    jnz %s_%s\n", generator->current_function_name, instr->label);
        break;

    case IR_JUMP_IF_FALSE:
        fprintf(generator->output_file, "    cmp qword [rel %s], 0\n",
                codegenasm_get_operand_name(generator, instr->arg1));
        fprintf(generator->output_file, "    jz %s_%s\n", generator->current_function_name, instr->label);
        break;

    case IR_PARAM:
        if (generator->param_count < MAX_PARAMS)
        {
            generator->params[generator->param_count++] = instr->arg1;
        }
        break;

    case IR_CALL:
        codegenasm_call(generator, instr->result, instr->label);
        break;

    case IR_RETURN:
        if (instr->arg1)
        {
            fprintf(generator->output_file, "    mov rax, qword [rel %s]\n",
                    codegenasm_get_operand_name(generator, instr->arg1));
        }
        else
        {
            fprintf(generator->output_file, "    mov rax, 0\n");
        }
        fprintf(generator->output_file, "    jmp %s\n", generator->epilogue_label);
        break;

    case IR_PRINT:
        codegenasm_print(generator, instr->arg1);
        break;
    case IR_ARRAY_LOAD:
        codegenasm_array_load(generator, instr->result, instr->arg1, instr->arg2);
        break;
    case IR_ARRAY_STORE:
        codegenasm_array_store(generator, instr->arg1, instr->arg2, instr->result);
        break;
    case IR_BOUNDS_CHECK:
        codegenasm_bounds_check(generator, instr->arg1, instr->arg2, instr->label);
        break;
    case IR_ARRAY_DECL:
        break;
    case IR_ARRAY_INIT:
        break;
    case IR_VAR_DECL:
        break;
    }
}

void codegenasm_move(CodeGenerator *generator, IROperand *dest, IROperand *src)
{
    char *src_name = codegenasm_get_operand_name(generator, src);
    char *dest_name = codegenasm_get_operand_name(generator, dest);
    if ((strcmp(src_name, "rcx") == 0 || strcmp(src_name, "rdx") == 0 || strcmp(src_name, "r8") == 0 || strcmp(src_name, "r9") == 0) &&
        (strcmp(dest_name, "rcx") == 0 || strcmp(dest_name, "rdx") == 0 || strcmp(dest_name, "r8") == 0 || strcmp(dest_name, "r9") == 0))
    {
        fprintf(generator->output_file, "    mov %s, %s\n", dest_name, src_name);
    }
    else if (strcmp(dest_name, "rcx") == 0 || strcmp(dest_name, "rdx") == 0 || strcmp(dest_name, "r8") == 0 || strcmp(dest_name, "r9") == 0)
    {
        fprintf(generator->output_file, "    mov %s, qword [rel %s]\n", dest_name, src_name);
    }
    else if (strcmp(src_name, "rcx") == 0 || strcmp(src_name, "rdx") == 0 || strcmp(src_name, "r8") == 0 || strcmp(src_name, "r9") == 0)
    {
        fprintf(generator->output_file, "    mov qword [rel %s], %s\n", dest_name, src_name);
    }
    else
    {
        fprintf(generator->output_file, "    mov rax, qword [rel %s]\n", src_name);
        fprintf(generator->output_file, "    mov qword [rel %s], rax\n", dest_name);
    }
}

void codegenasm_binary_op(CodeGenerator *generator, const char *op, IROperand *result, IROperand *arg1, IROperand *arg2)
{
    char *arg1_name = codegenasm_get_operand_name(generator, arg1);
    char *arg2_name = codegenasm_get_operand_name(generator, arg2);
    char *result_name = codegenasm_get_operand_name(generator, result);
    if (strcmp(arg1_name, "rcx") == 0 || strcmp(arg1_name, "rdx") == 0 || strcmp(arg1_name, "r8") == 0 || strcmp(arg1_name, "r9") == 0)
    {
        fprintf(generator->output_file, "    mov rax, %s\n", arg1_name);
    }
    else
    {
        fprintf(generator->output_file, "    mov rax, qword [rel %s]\n", arg1_name);
    }
    if (strcmp(arg2_name, "rcx") == 0 || strcmp(arg2_name, "rdx") == 0 || strcmp(arg2_name, "r8") == 0 || strcmp(arg2_name, "r9") == 0)
    {
        fprintf(generator->output_file, "    %s rax, %s\n", op, arg2_name);
    }
    else
    {
        fprintf(generator->output_file, "    %s rax, qword [rel %s]\n", op, arg2_name);
    }
    fprintf(generator->output_file, "    mov qword [rel %s], rax\n", result_name);
}

void codegenasm_unary_op(CodeGenerator *generator, const char *op, IROperand *result, IROperand *arg)
{
    fprintf(generator->output_file, "    mov rax, qword [rel %s]\n",
            codegenasm_get_operand_name(generator, arg));
    fprintf(generator->output_file, "    %s rax\n", op);
    fprintf(generator->output_file, "    mov qword [rel %s], rax\n",
            codegenasm_get_operand_name(generator, result));
}

void codegenasm_mul(CodeGenerator *generator, IROperand *result, IROperand *arg1, IROperand *arg2)
{
    char *arg1_name = codegenasm_get_operand_name(generator, arg1);
    char *arg2_name = codegenasm_get_operand_name(generator, arg2);
    char *result_name = codegenasm_get_operand_name(generator, result);

    if (arg1->type == IR_OP_VAR && generator->current_function_name)
    {
        IRFunction *func = NULL;
        for (size_t i = 0; i < generator->ir_program->functions.size; i++)
        {
            IRFunction *f = (IRFunction *)array_get(&generator->ir_program->functions, i);
            if (string_equal(f->name, generator->current_function_name))
            {
                func = f;
                break;
            }
        }
        if (func)
        {
            for (size_t i = 0; i < func->params.size; i++)
            {
                IROperand *param = (IROperand *)array_get(&func->params, i);
                if (string_equal(param->data.var_name, arg1->data.var_name))
                {
                    fprintf(generator->output_file, "    mov rax, qword [rel %s_param]\n", generator->current_function_name);
                    goto arg2_handling;
                }
            }
        }
    }

    if (is_param_register(arg1_name))
    {
        fprintf(generator->output_file, "    mov rax, %s\n", arg1_name);
    }
    else
    {
        fprintf(generator->output_file, "    mov rax, qword [rel %s]\n", arg1_name);
    }

arg2_handling:
    if (is_param_register(arg2_name))
    {
        fprintf(generator->output_file, "    imul rax, %s\n", arg2_name);
    }
    else
    {
        fprintf(generator->output_file, "    imul rax, qword [rel %s]\n", arg2_name);
    }
    fprintf(generator->output_file, "    mov qword [rel %s], rax\n", result_name);
}

void codegenasm_div(CodeGenerator *generator, IROperand *result, IROperand *arg1, IROperand *arg2)
{
    char *arg1_name = codegenasm_get_operand_name(generator, arg1);
    char *arg2_name = codegenasm_get_operand_name(generator, arg2);
    char *result_name = codegenasm_get_operand_name(generator, result);
    if (is_param_register(arg1_name))
    {
        fprintf(generator->output_file, "    mov rax, %s\n", arg1_name);
    }
    else
    {
        fprintf(generator->output_file, "    mov rax, qword [rel %s]\n", arg1_name);
    }
    fprintf(generator->output_file, "    cqo\n");
    if (is_param_register(arg2_name))
    {
        fprintf(generator->output_file, "    idiv %s\n", arg2_name);
    }
    else
    {
        fprintf(generator->output_file, "    idiv qword [rel %s]\n", arg2_name);
    }
    fprintf(generator->output_file, "    mov qword [rel %s], rax\n", result_name);
}

void codegenasm_mod(CodeGenerator *generator, IROperand *result, IROperand *arg1, IROperand *arg2)
{
    char *arg1_name = codegenasm_get_operand_name(generator, arg1);
    char *arg2_name = codegenasm_get_operand_name(generator, arg2);
    char *result_name = codegenasm_get_operand_name(generator, result);
    if (is_param_register(arg1_name))
    {
        fprintf(generator->output_file, "    mov rax, %s\n", arg1_name);
    }
    else
    {
        fprintf(generator->output_file, "    mov rax, qword [rel %s]\n", arg1_name);
    }
    fprintf(generator->output_file, "    cqo\n");
    if (is_param_register(arg2_name))
    {
        fprintf(generator->output_file, "    idiv %s\n", arg2_name);
    }
    else
    {
        fprintf(generator->output_file, "    idiv qword [rel %s]\n", arg2_name);
    }
    fprintf(generator->output_file, "    mov qword [rel %s], rdx\n", result_name);
}

void codegenasm_not(CodeGenerator *generator, IROperand *result, IROperand *arg)
{
    fprintf(generator->output_file, "    mov rax, qword [rel %s]\n",
            codegenasm_get_operand_name(generator, arg));
    fprintf(generator->output_file, "    not rax\n");
    fprintf(generator->output_file, "    mov qword [rel %s], rax\n",
            codegenasm_get_operand_name(generator, result));
}

void codegenasm_compare(CodeGenerator *generator, const char *set_op, IROperand *result, IROperand *arg1, IROperand *arg2)
{
    char *arg1_name = codegenasm_get_operand_name(generator, arg1);
    char *arg2_name = codegenasm_get_operand_name(generator, arg2);
    char *result_name = codegenasm_get_operand_name(generator, result);
    if (is_param_register(arg1_name))
    {
        fprintf(generator->output_file, "    mov rax, %s\n", arg1_name);
    }
    else
    {
        fprintf(generator->output_file, "    mov rax, qword [rel %s]\n", arg1_name);
    }
    if (is_param_register(arg2_name))
    {
        fprintf(generator->output_file, "    cmp rax, %s\n", arg2_name);
    }
    else
    {
        fprintf(generator->output_file, "    cmp rax, qword [rel %s]\n", arg2_name);
    }
    fprintf(generator->output_file, "    %s al\n", set_op);
    fprintf(generator->output_file, "    movzx rax, al\n");
    fprintf(generator->output_file, "    mov qword [rel %s], rax\n", result_name);
}

void codegenasm_call(CodeGenerator *generator, IROperand *result, const char *func_name)
{
    fprintf(generator->output_file, "    sub rsp, 40\n");
    const char *regs[] = {"rcx", "rdx", "r8", "r9"};
    for (int i = 0; i < generator->param_count && i < 4; i++)
    {
        fprintf(generator->output_file, "    mov %s, qword [rel %s]\n",
                regs[i], codegenasm_get_operand_name(generator, generator->params[i]));
    }
    fprintf(generator->output_file, "    call %s\n", func_name);
    fprintf(generator->output_file, "    add rsp, 40\n");
    if (result)
    {
        fprintf(generator->output_file, "    mov qword [rel %s], rax\n",
                codegenasm_get_operand_name(generator, result));
    }
    generator->param_count = 0;
}

void codegenasm_print(CodeGenerator *generator, IROperand *value)
{
    fprintf(generator->output_file, "    sub rsp, 40\n");
    fprintf(generator->output_file, "    lea rcx, [rel format_int]\n");
    char *value_name = codegenasm_get_operand_name(generator, value);
    if (is_param_register(value_name))
    {
        fprintf(generator->output_file, "    mov rdx, %s\n", value_name);
    }
    else
    {
        fprintf(generator->output_file, "    mov rdx, qword [rel %s]\n", value_name);
    }
    fprintf(generator->output_file, "    mov rax, qword [rel __imp_printf]\n");
    fprintf(generator->output_file, "    call rax\n");
    fprintf(generator->output_file, "    add rsp, 40\n");
}

void codegenasm_write_header(CodeGenerator *generator)
{
    if (debug_enabled)
    {
        printf("[DEBUG] Wrote header\n");
        fflush(stdout);
    }
    fprintf(generator->output_file, "; Generated assembly code for .tl language\n");
    fprintf(generator->output_file, "; Target: x86-64 Windows\n\n");
    fprintf(generator->output_file, "extern __imp_printf\n");
    fprintf(generator->output_file, "extern __imp_ExitProcess\n\n");
}

void codegenasm_write_data_section(CodeGenerator *generator)
{
    if (debug_enabled)
    {
        printf("[DEBUG] Entered codegenasm_write_data_section\n");
        fflush(stdout);
    }
    fprintf(generator->output_file, "section .data\n");
    fprintf(generator->output_file, "format_int: db \"%%ld\", 10, 0\n");

    fprintf(generator->output_file, "const_0: dq 0\n");
    fprintf(generator->output_file, "const_1: dq 1\n");
    fprintf(generator->output_file, "const_2: dq 2\n");
    fprintf(generator->output_file, "const_4: dq 4\n");
    fprintf(generator->output_file, "const_5: dq 5\n");
    fprintf(generator->output_file, "const_6: dq 6\n");
    fprintf(generator->output_file, "const_10: dq 10\n");
    fprintf(generator->output_file, "const_15: dq 15\n");
    fprintf(generator->output_file, "const_17: dq 17\n");
    fprintf(generator->output_file, "const_18: dq 18\n");
    fprintf(generator->output_file, "const_42: dq 42\n");
    fprintf(generator->output_file, "const_48: dq 48\n");

    if (debug_enabled)
    {
        printf("[DEBUG] Wrote format_int\n");
        fflush(stdout);
    }

    for (size_t i = 0; i < generator->ir_program->functions.size; i++)
    {
        IRFunction *func = (IRFunction *)array_get(&generator->ir_program->functions, i);
        if (debug_enabled)
        {
            printf("[DEBUG] Processing function %s for data section\n", func->name);
            fflush(stdout);
        }
        if (debug_enabled)
        {
            printf("[DEBUG] Function %s has %zu instructions\n", func->name, func->instructions.size);
            fflush(stdout);
        }
        generator->current_function_name = func->name;
        fprintf(generator->output_file, "%s_param: dq 0\n", func->name);
        for (size_t j = 0; j < func->instructions.size; j++)
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Processing instruction %zu in function %s\n", j, func->name);
                fflush(stdout);
            }
            IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, j);
            if (debug_enabled)
            {
                printf("[DEBUG] Instruction opcode: %s\n", ir_opcode_to_string(instr->opcode));
                fflush(stdout);
            }
            if (instr->result && instr->result->type == IR_OP_TEMP)
            {
                if (debug_enabled)
                {
                    printf("[DEBUG] Processing result temp\n");
                    fflush(stdout);
                }
                char *temp_name = codegenasm_get_temp_name(generator, instr->result);
                if (!hashtable_get(generator->declared_temps, temp_name))
                {
                    fprintf(generator->output_file, "%s: dq 0\n", temp_name);
                    hashtable_put(generator->declared_temps, temp_name, (void *)1);
                    if (debug_enabled)
                    {
                        printf("[DEBUG] Declared temp: %s\n", temp_name);
                        fflush(stdout);
                    }
                }
                else
                {
                    if (debug_enabled)
                    {
                        printf("[DEBUG] Temp already declared: %s\n", temp_name);
                        fflush(stdout);
                    }
                }
            }
            if (instr->result && instr->result->type == IR_OP_VAR)
            {
                char *var_name = instr->result->data.var_name;
                bool is_param = false;
                for (size_t k = 0; k < func->params.size; k++)
                {
                    IROperand *param = (IROperand *)array_get(&func->params, k);
                    if (string_equal(var_name, param->data.var_name))
                    {
                        is_param = true;
                        break;
                    }
                }
                if (!is_param && !hashtable_get(generator->declared_temps, var_name))
                {
                    fprintf(generator->output_file, "%s: dq 0\n", var_name);
                    hashtable_put(generator->declared_temps, var_name, (void *)1);
                    if (debug_enabled)
                    {
                        printf("[DEBUG] Declared var: %s\n", var_name);
                        fflush(stdout);
                    }
                }
            }
            if (instr->arg1 && instr->arg1->type == IR_OP_TEMP)
            {
                if (debug_enabled)
                {
                    printf("[DEBUG] Processing arg1 temp\n");
                    fflush(stdout);
                }
                char *temp_name = codegenasm_get_temp_name(generator, instr->arg1);
                if (!hashtable_get(generator->declared_temps, temp_name))
                {
                    fprintf(generator->output_file, "%s: dq 0\n", temp_name);
                    hashtable_put(generator->declared_temps, temp_name, (void *)1);
                    if (debug_enabled)
                    {
                        printf("[DEBUG] Declared temp: %s\n", temp_name);
                        fflush(stdout);
                    }
                }
                else
                {
                    if (debug_enabled)
                    {
                        printf("[DEBUG] Temp already declared: %s\n", temp_name);
                        fflush(stdout);
                    }
                }
            }
            if (instr->arg1 && instr->arg1->type == IR_OP_VAR)
            {
                char *var_name = instr->arg1->data.var_name;
                bool is_param = false;
                for (size_t k = 0; k < func->params.size; k++)
                {
                    IROperand *param = (IROperand *)array_get(&func->params, k);
                    if (string_equal(var_name, param->data.var_name))
                    {
                        is_param = true;
                        break;
                    }
                }
                if (!is_param && !hashtable_get(generator->declared_temps, var_name))
                {
                    fprintf(generator->output_file, "%s: dq 0\n", var_name);
                    hashtable_put(generator->declared_temps, var_name, (void *)1);
                    if (debug_enabled)
                    {
                        printf("[DEBUG] Declared var: %s\n", var_name);
                        fflush(stdout);
                    }
                }
            }
            if (instr->arg2 && instr->arg2->type == IR_OP_TEMP)
            {
                if (debug_enabled)
                {
                    printf("[DEBUG] Processing arg2 temp\n");
                    fflush(stdout);
                }
                char *temp_name = codegenasm_get_temp_name(generator, instr->arg2);
                if (!hashtable_get(generator->declared_temps, temp_name))
                {
                    fprintf(generator->output_file, "%s: dq 0\n", temp_name);
                    hashtable_put(generator->declared_temps, temp_name, (void *)1);
                    if (debug_enabled)
                    {
                        printf("[DEBUG] Declared temp: %s\n", temp_name);
                        fflush(stdout);
                    }
                }
                else
                {
                    if (debug_enabled)
                    {
                        printf("[DEBUG] Temp already declared: %s\n", temp_name);
                        fflush(stdout);
                    }
                }
            }
            if (instr->arg2 && instr->arg2->type == IR_OP_VAR)
            {
                char *var_name = instr->arg2->data.var_name;
                bool is_param = false;
                for (size_t k = 0; k < func->params.size; k++)
                {
                    IROperand *param = (IROperand *)array_get(&func->params, k);
                    if (string_equal(var_name, param->data.var_name))
                    {
                        is_param = true;
                        break;
                    }
                }
                if (!is_param && !hashtable_get(generator->declared_temps, var_name))
                {
                    fprintf(generator->output_file, "%s: dq 0\n", var_name);
                    hashtable_put(generator->declared_temps, var_name, (void *)1);
                    if (debug_enabled)
                    {
                        printf("[DEBUG] Declared var: %s\n", var_name);
                        fflush(stdout);
                    }
                }
            }
        }
        if (debug_enabled)
        {
            printf("[DEBUG] Finished processing function %s\n", func->name);
            fflush(stdout);
        }
    }
    if (debug_enabled)
    {
        printf("[DEBUG] Exiting codegenasm_write_data_section\n");
        fflush(stdout);
    }
}

void codegenasm_write_text_section(CodeGenerator *generator)
{
    if (debug_enabled)
    {
        printf("[DEBUG] Wrote data section\n");
        fflush(stdout);
    }
    fprintf(generator->output_file, "\nsection .text\n");
    fprintf(generator->output_file, "global _start\n\n");

    fprintf(generator->output_file, "_start:\n");
    fprintf(generator->output_file, "    call main\n");
    fprintf(generator->output_file, "    mov rcx, rax\n");
    fprintf(generator->output_file, "    mov rax, qword [rel __imp_ExitProcess]\n");
    fprintf(generator->output_file, "    jmp rax\n\n");
}

void codegenasm_write_function_header(CodeGenerator *generator, IRFunction *func)
{
    fprintf(generator->output_file, "\n; Function: %s\n", func->name);
    fprintf(generator->output_file, "%s:\n", func->name);

    // Stack alignment for x64 Windows calling convention
    // Reserve 32 bytes of shadow space + align stack to 16-byte boundary
    fprintf(generator->output_file, "    sub rsp, 40\n");

    fprintf(generator->output_file, "    push rbx\n");
    fprintf(generator->output_file, "    push rsi\n");
    fprintf(generator->output_file, "    push rdi\n");
    fprintf(generator->output_file, "    push r12\n");
    fprintf(generator->output_file, "    push r13\n");
    fprintf(generator->output_file, "    push r14\n");
    fprintf(generator->output_file, "    push r15\n");
    fprintf(generator->output_file, "    mov rbx, qword [rel %s_param]\n", func->name);
    fprintf(generator->output_file, "    mov qword [rel %s_param], rcx\n", func->name);
}

void codegenasm_write_function_footer(CodeGenerator *generator)
{
    fprintf(generator->output_file, "    mov qword [rel %s_param], rbx\n", generator->current_function_name);
    fprintf(generator->output_file, "    pop r15\n");
    fprintf(generator->output_file, "    pop r14\n");
    fprintf(generator->output_file, "    pop r13\n");
    fprintf(generator->output_file, "    pop r12\n");
    fprintf(generator->output_file, "    pop rdi\n");
    fprintf(generator->output_file, "    pop rsi\n");
    fprintf(generator->output_file, "    pop rbx\n");

    fprintf(generator->output_file, "    add rsp, 40\n");
    fprintf(generator->output_file, "    ret\n");
}

void codegenasm_write_main_function(CodeGenerator *generator)
{
    fprintf(generator->output_file, "\n; Main function\n");
    fprintf(generator->output_file, "_start:\n");
    fprintf(generator->output_file, "    call main\n");
    fprintf(generator->output_file, "    mov rcx, rax\n");
    fprintf(generator->output_file, "    mov rax, qword [rel __imp_ExitProcess]\n");
    fprintf(generator->output_file, "    jmp rax\n");
}

void codegenasm_error(CodeGenerator *generator, const char *message)
{
    if (generator->error)
    {
        error_set(generator->error, ERROR_CODEGEN, message, 0, 0);
    }
}

void codegenasm_array_load(CodeGenerator *generator, IROperand *result, IROperand *array, IROperand *index)
{
    char *array_name = codegenasm_get_operand_name(generator, array);
    char *index_name = codegenasm_get_operand_name(generator, index);
    char *result_name = codegenasm_get_operand_name(generator, result);

    fprintf(generator->output_file, "    lea rax, [rel %s]\n", array_name);
    if (is_param_register(index_name))
    {
        fprintf(generator->output_file, "    mov rcx, %s\n", index_name);
    }
    else
    {
        fprintf(generator->output_file, "    mov rcx, qword [rel %s]\n", index_name);
    }

    fprintf(generator->output_file, "    imul rcx, 8\n");
    fprintf(generator->output_file, "    add rax, rcx\n");
    fprintf(generator->output_file, "    mov rax, qword [rax]\n");
    fprintf(generator->output_file, "    mov qword [rel %s], rax\n", result_name);
}

void codegenasm_array_store(CodeGenerator *generator, IROperand *array, IROperand *index, IROperand *value)
{
    char *array_name = codegenasm_get_operand_name(generator, array);
    char *index_name = codegenasm_get_operand_name(generator, index);
    char *value_name = codegenasm_get_operand_name(generator, value);

    fprintf(generator->output_file, "    lea rax, [rel %s]\n", array_name);
    if (is_param_register(index_name))
    {
        fprintf(generator->output_file, "    mov rcx, %s\n", index_name);
    }
    else
    {
        fprintf(generator->output_file, "    mov rcx, qword [rel %s]\n", index_name);
    }

    fprintf(generator->output_file, "    imul rcx, 8\n");
    fprintf(generator->output_file, "    add rax, rcx\n");

    if (is_param_register(value_name))
    {
        fprintf(generator->output_file, "    mov rcx, %s\n", value_name);
    }
    else
    {
        fprintf(generator->output_file, "    mov rcx, qword [rel %s]\n", value_name);
    }

    fprintf(generator->output_file, "    mov qword [rax], rcx\n");
}

void codegenasm_bounds_check(CodeGenerator *generator, IROperand *index, IROperand *size, const char *error_label)
{
    char *index_name = codegenasm_get_operand_name(generator, index);
    char *size_name = codegenasm_get_operand_name(generator, size);

    if (is_param_register(index_name))
    {
        fprintf(generator->output_file, "    mov rax, %s\n", index_name);
    }
    else
    {
        fprintf(generator->output_file, "    mov rax, qword [rel %s]\n", index_name);
    }

    if (is_param_register(size_name))
    {
        fprintf(generator->output_file, "    mov rcx, %s\n", size_name);
    }
    else
    {
        fprintf(generator->output_file, "    mov rcx, qword [rel %s]\n", size_name);
    }

    fprintf(generator->output_file, "    cmp rax, rcx\n");
    fprintf(generator->output_file, "    jge %s_%s\n", generator->current_function_name, error_label);
}