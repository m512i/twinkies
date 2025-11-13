#include "backend/ir/ir.h"
#include "analysis/semantic/semantic.h"
#include "backend/ir/iroperands.h"
#include "backend/ir/irinstructions.h"
#include "common/common.h"
#include "modules/modules.h"

extern bool debug_enabled;

LoopContext *ir_loop_context_create(char *start_label, char *end_label)
{
    LoopContext *context = safe_malloc(sizeof(LoopContext));
    context->start_label = string_copy(start_label);
    context->end_label = string_copy(end_label);
    context->parent = NULL;
    return context;
}

void ir_loop_context_destroy(LoopContext *context)
{
    if (!context)
        return;

    safe_free(context->start_label);
    safe_free(context->end_label);
    safe_free(context);
}

void ir_function_enter_loop(IRFunction *func, char *start_label, char *end_label)
{
    LoopContext *new_context = ir_loop_context_create(start_label, end_label);
    if (func->loop_stack.size > 0) {
        new_context->parent = (LoopContext *)array_get(&func->loop_stack, func->loop_stack.size - 1);
    } else {
        new_context->parent = NULL;
    }
    array_push(&func->loop_stack, new_context);
}

void ir_function_exit_loop(IRFunction *func)
{
    if (func->loop_stack.size == 0)
        return;

    LoopContext *current = (LoopContext *)array_get(&func->loop_stack, func->loop_stack.size - 1);
    func->loop_stack.size--;
    ir_loop_context_destroy(current);
}

LoopContext *ir_function_get_current_loop(IRFunction *func)
{
    if (func->loop_stack.size == 0)
        return NULL;
    return (LoopContext *)array_get(&func->loop_stack, func->loop_stack.size - 1);
}

IRFunction *ir_function_create(const char *name, DataType return_type)
{
    IRFunction *func = safe_malloc(sizeof(IRFunction));
    func->name = string_copy(name);
    func->return_type = return_type;
    array_init(&func->params, 4);
    array_init(&func->instructions, 16);
    func->temp_counter = 0;
    func->label_counter = 0;
    array_init(&func->loop_stack, sizeof(LoopContext *));
    func->oob_error_label = NULL;
    return func;
}

IRProgram *ir_program_create(void)
{
    IRProgram *program = safe_malloc(sizeof(IRProgram));
    array_init(&program->functions, 4);
    return program;
}

void ir_function_add_instruction(IRFunction *func, IRInstruction *instr)
{
    array_push(&func->instructions, instr);
}

void ir_function_add_param(IRFunction *func, IROperand *param)
{
    array_push(&func->params, param);
}

void ir_program_add_function(IRProgram *program, IRFunction *func)
{
    array_push(&program->functions, func);
}

void ir_function_destroy(IRFunction *func)
{
    if (!func)
        return;

    safe_free(func->name);

    for (size_t i = 0; i < func->params.size; i++)
    {
        ir_operand_destroy((IROperand *)array_get(&func->params, i));
    }
    array_free(&func->params);

    for (size_t i = 0; i < func->instructions.size; i++)
    {
        ir_instruction_destroy((IRInstruction *)array_get(&func->instructions, i));
    }
    array_free(&func->instructions);

    while (func->loop_stack.size > 0)
    {
        ir_function_exit_loop(func);
    }
    array_free(&func->loop_stack);

    safe_free(func->oob_error_label);
    safe_free(func);
}

void ir_program_destroy(IRProgram *program)
{
    if (!program)
        return;

    for (size_t i = 0; i < program->functions.size; i++)
    {
        ir_function_destroy((IRFunction *)array_get(&program->functions, i));
    }
    array_free(&program->functions);
    safe_free(program);
}

void ir_function_print(const IRFunction *func)
{
    if (!func)
        return;

    printf("Function: %s\n", func->name);
    printf("Parameters: ");
    for (size_t i = 0; i < func->params.size; i++)
    {
        if (i > 0)
            printf(", ");
        ir_operand_print((IROperand *)array_get(&func->params, i));
    }
    printf("\n");

    printf("Instructions:\n");
    for (size_t i = 0; i < func->instructions.size; i++)
    {
        printf("  %zu: ", i);
        ir_instruction_print((IRInstruction *)array_get(&func->instructions, i));
    }
    printf("\n");
}

void ir_program_print(const IRProgram *program)
{
    if (!program)
        return;

    printf("IR Program:\n");
    printf("===========\n");

    for (size_t i = 0; i < program->functions.size; i++)
    {
        ir_function_print((IRFunction *)array_get(&program->functions, i));
    }
}

const char *ir_opcode_to_string(IROpcode opcode)
{
    switch (opcode)
    {
    case IR_NOP:
        return "NOP";
    case IR_LABEL:
        return "LABEL";
    case IR_MOVE:
        return "MOVE";
    case IR_ADD:
        return "+";
    case IR_SUB:
        return "-";
    case IR_MUL:
        return "*";
    case IR_DIV:
        return "/";
    case IR_MOD:
        return "%";
    case IR_NEG:
        return "NEG";
    case IR_NOT:
        return "NOT";
    case IR_EQ:
        return "==";
    case IR_NE:
        return "!=";
    case IR_LT:
        return "<";
    case IR_LE:
        return "<=";
    case IR_GT:
        return ">";
    case IR_GE:
        return ">=";
    case IR_AND:
        return "&&";
    case IR_OR:
        return "||";
    case IR_JUMP:
        return "JUMP";
    case IR_JUMP_IF:
        return "JUMP_IF";
    case IR_JUMP_IF_FALSE:
        return "JUMP_IF_FALSE";
    case IR_CALL:
        return "CALL";
    case IR_RETURN:
        return "RETURN";
    case IR_PARAM:
        return "PARAM";
    case IR_PRINT:
        return "PRINT";
    case IR_ARRAY_LOAD:
        return "ARRAY_LOAD";
    case IR_ARRAY_STORE:
        return "ARRAY_STORE";
    case IR_BOUNDS_CHECK:
        return "BOUNDS_CHECK";
    case IR_ARRAY_DECL:
        return "ARRAY_DECL";
    case IR_ARRAY_INIT:
        return "ARRAY_INIT";
    case IR_VAR_DECL:
        return "VAR_DECL";
    case IR_INLINE_ASM:
        return "INLINE_ASM";
    default:
        return "UNKNOWN";
    }
}

int ir_function_new_temp(IRFunction *func)
{
    return func->temp_counter++;
}

char *ir_function_new_label(IRFunction *func)
{
    char *label = safe_malloc(32);
    snprintf(label, 32, "L%d", func->label_counter++);
    return label;
}
