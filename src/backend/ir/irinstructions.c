#include "backend/ir/irinstructions.h"
#include "backend/ir/iroperands.h"

IRInstruction *ir_instruction_nop(void)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_NOP;
    instr->result = NULL;
    instr->arg1 = NULL;
    instr->arg2 = NULL;
    instr->args = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction *ir_instruction_label(const char *label)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_LABEL;
    instr->result = NULL;
    instr->arg1 = NULL;
    instr->arg2 = NULL;
    instr->args = NULL;
    instr->label = string_copy(label);
    return instr;
}

IRInstruction *ir_instruction_move(IROperand *result, IROperand *source)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_MOVE;
    instr->result = result;
    instr->arg1 = source;
    instr->arg2 = NULL;
    instr->args = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction *ir_instruction_binary(IROpcode opcode, IROperand *result, IROperand *arg1, IROperand *arg2)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = opcode;
    instr->result = result;
    instr->arg1 = arg1;
    instr->arg2 = arg2;
    instr->args = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction *ir_instruction_unary(IROpcode opcode, IROperand *result, IROperand *arg)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = opcode;
    instr->result = result;
    instr->arg1 = arg;
    instr->arg2 = NULL;
    instr->args = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction *ir_instruction_jump(const char *label)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_JUMP;
    instr->result = NULL;
    instr->arg1 = NULL;
    instr->arg2 = NULL;
    instr->args = NULL;
    instr->label = string_copy(label);
    return instr;
}

IRInstruction *ir_instruction_jump_if(IROperand *condition, const char *label)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_JUMP_IF;
    instr->result = NULL;
    instr->arg1 = condition;
    instr->arg2 = NULL;
    instr->args = NULL;
    instr->label = string_copy(label);
    return instr;
}

IRInstruction *ir_instruction_jump_if_false(IROperand *condition, const char *label)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_JUMP_IF_FALSE;
    instr->result = NULL;
    instr->arg1 = condition;
    instr->arg2 = NULL;
    instr->args = NULL;
    instr->label = string_copy(label);
    return instr;
}

IRInstruction *ir_instruction_call(IROperand *result, const char *func_name)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_CALL;
    instr->result = result;
    instr->arg1 = NULL;
    instr->arg2 = NULL;
    instr->args = NULL;
    instr->label = string_copy(func_name);
    return instr;
}

IRInstruction *ir_instruction_return(IROperand *value)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_RETURN;
    instr->result = NULL;
    instr->arg1 = value;
    instr->arg2 = NULL;
    instr->args = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction *ir_instruction_param(IROperand *param)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_PARAM;
    instr->result = NULL;
    instr->arg1 = param;
    instr->arg2 = NULL;
    instr->args = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction *ir_instruction_print_op(IROperand *value)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_PRINT;
    instr->result = NULL;
    instr->arg1 = value;
    instr->arg2 = NULL;
    instr->args = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction *ir_instruction_print_multiple(DynamicArray *args)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_PRINT;
    instr->result = NULL;
    instr->arg1 = NULL;
    instr->arg2 = NULL;
    instr->args = args;
    instr->label = NULL;
    return instr;
}

IRInstruction *ir_instruction_array_load(IROperand *result, IROperand *array, IROperand *index)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_ARRAY_LOAD;
    instr->result = result;
    instr->arg1 = array;
    instr->arg2 = index;
    instr->args = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction *ir_instruction_array_store(IROperand *array, IROperand *index, IROperand *value)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_ARRAY_STORE;
    instr->result = value;
    instr->arg1 = array;
    instr->arg2 = index;
    instr->args = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction *ir_instruction_bounds_check(IROperand *index, IROperand *size, const char *error_label)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_BOUNDS_CHECK;
    instr->result = NULL;
    instr->arg1 = index;
    instr->arg2 = size;
    instr->args = NULL;
    instr->label = string_copy(error_label);
    return instr;
}

IRInstruction *ir_instruction_array_decl(const char *array_name, int size, DataType element_type)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_ARRAY_DECL;
    IROperand *array_var = ir_operand_array_var(array_name, size);
    array_var->data_type = element_type;
    instr->result = array_var;
    instr->arg1 = NULL;
    instr->arg2 = NULL;
    instr->args = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction *ir_instruction_array_init(const char *array_name, int size, DataType element_type, IROperand *value)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_ARRAY_INIT;
    IROperand *array_var = ir_operand_array_var(array_name, size);
    array_var->data_type = element_type;
    instr->result = array_var;
    instr->arg1 = value;
    instr->arg2 = NULL;
    instr->args = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction *ir_instruction_var_decl(const char *var_name, DataType type)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_VAR_DECL;
    IROperand *var = ir_operand_var(var_name);
    var->data_type = type;
    instr->result = var;
    instr->arg1 = NULL;
    instr->arg2 = NULL;
    instr->args = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction *ir_instruction_inline_asm(const char *asm_code, bool is_volatile, DynamicArray *outputs, DynamicArray *inputs, DynamicArray *clobbers)
{
    IRInstruction *instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_INLINE_ASM;
    instr->result = NULL;
    instr->arg1 = NULL;
    instr->arg2 = NULL;
    instr->label = NULL;
    instr->func_name = NULL;
    instr->array_name = NULL;
    instr->args = NULL;
    instr->asm_code = string_copy(asm_code);
    instr->asm_volatile = is_volatile;
    instr->asm_outputs = outputs;
    instr->asm_inputs = inputs;
    instr->asm_clobbers = clobbers;
    return instr;
}

void ir_instruction_destroy(IRInstruction *instr)
{
    if (!instr)
        return;

    if (instr->result)
        ir_operand_destroy(instr->result);
    if (instr->arg1)
        ir_operand_destroy(instr->arg1);
    if (instr->arg2)
        ir_operand_destroy(instr->arg2);
    if (instr->args)
    {
        for (size_t i = 0; i < instr->args->size; i++)
        {
            IROperand *arg = (IROperand *)array_get(instr->args, i);
            ir_operand_destroy(arg);
        }
        array_free(instr->args);
        safe_free(instr->args);
    }
    if (instr->label)
        safe_free(instr->label);
    if (instr->asm_code)
        safe_free(instr->asm_code);
    if (instr->asm_outputs)
    {
        for (size_t i = 0; i < instr->asm_outputs->size; i++)
        {
            InlineAsmOperand *op = (InlineAsmOperand *)array_get(instr->asm_outputs, i);
            safe_free(op->constraint);
            safe_free(op->variable);
            safe_free(op);
        }
        array_free(instr->asm_outputs);
        safe_free(instr->asm_outputs);
    }
    if (instr->asm_inputs)
    {
        for (size_t i = 0; i < instr->asm_inputs->size; i++)
        {
            InlineAsmOperand *op = (InlineAsmOperand *)array_get(instr->asm_inputs, i);
            safe_free(op->constraint);
            safe_free(op->variable);
            safe_free(op);
        }
        array_free(instr->asm_inputs);
        safe_free(instr->asm_inputs);
    }
    if (instr->asm_clobbers)
    {
        for (size_t i = 0; i < instr->asm_clobbers->size; i++)
        {
            char *clobber = (char *)array_get(instr->asm_clobbers, i);
            safe_free(clobber);
        }
        array_free(instr->asm_clobbers);
        safe_free(instr->asm_clobbers);
    }
    safe_free(instr);
}

void ir_instruction_print(const IRInstruction *instr)
{
    if (!instr)
        return;

    switch (instr->opcode)
    {
    case IR_NOP:
        printf("NOP");
        break;
    case IR_LABEL:
        printf("%s:", instr->label);
        break;
    case IR_MOVE:
        ir_operand_print(instr->result);
        printf(" = ");
        ir_operand_print(instr->arg1);
        break;
    case IR_ADD:
    case IR_SUB:
    case IR_MUL:
    case IR_DIV:
    case IR_MOD:
    case IR_EQ:
    case IR_NE:
    case IR_LT:
    case IR_LE:
    case IR_GT:
    case IR_GE:
    case IR_AND:
    case IR_OR:
        ir_operand_print(instr->result);
        printf(" = ");
        ir_operand_print(instr->arg1);
        printf(" %s ", ir_opcode_to_string(instr->opcode));
        ir_operand_print(instr->arg2);
        break;
    case IR_NEG:
    case IR_NOT:
        ir_operand_print(instr->result);
        printf(" = %s ", ir_opcode_to_string(instr->opcode));
        ir_operand_print(instr->arg1);
        break;
    case IR_JUMP:
        printf("GOTO %s", instr->label);
        break;
    case IR_JUMP_IF:
        printf("IF ");
        ir_operand_print(instr->arg1);
        printf(" GOTO %s", instr->label);
        break;
    case IR_JUMP_IF_FALSE:
        printf("IF_FALSE ");
        ir_operand_print(instr->arg1);
        printf(" GOTO %s", instr->label);
        break;
    case IR_CALL:
        if (instr->result)
        {
            ir_operand_print(instr->result);
            printf(" = ");
        }
        printf("CALL %s", instr->label);
        break;
    case IR_RETURN:
        printf("RETURN");
        if (instr->arg1)
        {
            printf(" ");
            ir_operand_print(instr->arg1);
        }
        break;
    case IR_PARAM:
        printf("PARAM ");
        ir_operand_print(instr->arg1);
        break;
    case IR_PRINT:
        printf("PRINT ");
        ir_operand_print(instr->arg1);
        break;
    case IR_PRINT_MULTIPLE:
        printf("PRINT_MULTIPLE");
        if (instr->args)
        {
            for (size_t i = 0; i < instr->args->size; i++)
            {
                printf(" ");
                ir_operand_print((IROperand *)array_get(instr->args, i));
            }
        }
        break;
    case IR_INLINE_ASM:
        printf("INLINE_ASM");
        if (instr->asm_code)
        {
            printf(" \"%s\"", instr->asm_code);
        }
        break;
    case IR_ARRAY_LOAD:
        ir_operand_print(instr->result);
        printf(" = ");
        ir_operand_print(instr->arg1);
        printf("[");
        ir_operand_print(instr->arg2);
        printf("]");
        break;
    case IR_ARRAY_STORE:
        ir_operand_print(instr->arg1);
        printf("[");
        ir_operand_print(instr->arg2);
        printf("] = ");
        ir_operand_print(instr->result);
        break;
    case IR_BOUNDS_CHECK:
        printf("BOUNDS_CHECK ");
        ir_operand_print(instr->arg1);
        printf(" < ");
        ir_operand_print(instr->arg2);
        printf(" GOTO %s", instr->label);
        break;
    case IR_ARRAY_DECL:
        printf("ARRAY_DECL ");
        ir_operand_print(instr->result);
        break;
    case IR_ARRAY_INIT:
        printf("ARRAY_INIT ");
        ir_operand_print(instr->result);
        printf(" = ");
        ir_operand_print(instr->arg1);
        break;
    case IR_VAR_DECL:
        printf("VAR_DECL ");
        ir_operand_print(instr->result);
        break;
    }
    printf("\n");
}