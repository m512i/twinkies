#include "optimizations/optimizer.h"
#include "backend/ir/irCore.h"
#include "backend/ir/irOps.h"
#include "backend/ir/irinstructions.h"
#include "common/common.h"

extern bool debug_enabled;

static bool is_constant(IROperand *operand)
{
    return operand && operand->type == IR_OP_CONST;
}

static int64_t get_const_value(IROperand *operand)
{
    if (!is_constant(operand))
        return 0;
    return operand->data.const_value;
}

static double get_const_float_value(IROperand *operand)
{
    if (!is_constant(operand) || !operand->is_float_const)
        return 0.0;
    return operand->data.float_const_value;
}

static IROperand *fold_binary_op(IROpcode opcode, IROperand *arg1, IROperand *arg2, DataType result_type)
{
    if (!is_constant(arg1) || !is_constant(arg2))
        return NULL;

    bool is_float = arg1->is_float_const || arg2->is_float_const;
    int64_t result_int = 0;
    double result_float = 0.0;
    bool valid = true;

    if (is_float)
    {
        double val1 = get_const_float_value(arg1);
        double val2 = get_const_float_value(arg2);

        switch (opcode)
        {
        case IR_ADD:
            result_float = val1 + val2;
            break;
        case IR_SUB:
            result_float = val1 - val2;
            break;
        case IR_MUL:
            result_float = val1 * val2;
            break;
        case IR_DIV:
            if (val2 == 0.0)
                return NULL; 
            result_float = val1 / val2;
            break;
        case IR_EQ:
            result_int = (val1 == val2) ? 1 : 0;
            is_float = false;
            break;
        case IR_NE:
            result_int = (val1 != val2) ? 1 : 0;
            is_float = false;
            break;
        case IR_LT:
            result_int = (val1 < val2) ? 1 : 0;
            is_float = false;
            break;
        case IR_LE:
            result_int = (val1 <= val2) ? 1 : 0;
            is_float = false;
            break;
        case IR_GT:
            result_int = (val1 > val2) ? 1 : 0;
            is_float = false;
            break;
        case IR_GE:
            result_int = (val1 >= val2) ? 1 : 0;
            is_float = false;
            break;
        default:
            valid = false;
            break;
        }
    }
    else
    {
        int64_t val1 = get_const_value(arg1);
        int64_t val2 = get_const_value(arg2);

        switch (opcode)
        {
        case IR_ADD:
            result_int = val1 + val2;
            break;
        case IR_SUB:
            result_int = val1 - val2;
            break;
        case IR_MUL:
            result_int = val1 * val2;
            break;
        case IR_DIV:
            if (val2 == 0)
                return NULL; 
            result_int = val1 / val2;
            break;
        case IR_MOD:
            if (val2 == 0)
                return NULL; 
            result_int = val1 % val2;
            break;
        case IR_EQ:
            result_int = (val1 == val2) ? 1 : 0;
            break;
        case IR_NE:
            result_int = (val1 != val2) ? 1 : 0;
            break;
        case IR_LT:
            result_int = (val1 < val2) ? 1 : 0;
            break;
        case IR_LE:
            result_int = (val1 <= val2) ? 1 : 0;
            break;
        case IR_GT:
            result_int = (val1 > val2) ? 1 : 0;
            break;
        case IR_GE:
            result_int = (val1 >= val2) ? 1 : 0;
            break;
        case IR_AND:
            result_int = (val1 && val2) ? 1 : 0;
            break;
        case IR_OR:
            result_int = (val1 || val2) ? 1 : 0;
            break;
        default:
            valid = false;
            break;
        }
    }

    if (!valid)
        return NULL;

    if (is_float)
        return ir_operand_float_const(result_float);
    else
        return ir_operand_const(result_int);
}

static IROperand *fold_unary_op(IROpcode opcode, IROperand *arg, DataType result_type)
{
    if (!is_constant(arg))
        return NULL;

    bool is_float = arg->is_float_const;
    int64_t result_int = 0;
    double result_float = 0.0;

    if (is_float)
    {
        double val = get_const_float_value(arg);
        switch (opcode)
        {
        case IR_NEG:
            result_float = -val;
            break;
        default:
            return NULL;
        }
    }
    else
    {
        int64_t val = get_const_value(arg);
        switch (opcode)
        {
        case IR_NEG:
            result_int = -val;
            break;
        case IR_NOT:
            result_int = (!val) ? 1 : 0;
            break;
        default:
            return NULL;
        }
    }

    if (is_float)
        return ir_operand_float_const(result_float);
    else
        return ir_operand_const(result_int);
}

typedef struct {
    HashTable *constants; 
} ConstantPropagationState;

static ConstantPropagationState *cp_state_create(void)
{
    ConstantPropagationState *state = safe_malloc(sizeof(ConstantPropagationState));
    state->constants = hashtable_create(16);
    return state;
}

static void cp_state_destroy(ConstantPropagationState *state)
{
    if (!state)
        return;
    
    for (size_t i = 0; i < state->constants->capacity; i++)
    {
        HashTableEntry *entry = state->constants->buckets[i];
        while (entry)
        {
            IROperand *op = (IROperand *)entry->value;
            if (op)
            {
                ir_operand_destroy(op);
            }
            entry = entry->next;
        }
    }
    hashtable_destroy(state->constants);
    safe_free(state);
}

static bool optimize_function_constant_folding(IRFunction *func)
{
    bool changed = false;
    ConstantPropagationState *cp_state = cp_state_create();
    
    HashTable *loop_labels = hashtable_create(16);
    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (!instr)
            continue;
            
        if (instr->opcode == IR_JUMP && instr->label)
        {
            for (size_t j = 0; j < i; j++)
            {
                IRInstruction *prev_instr = (IRInstruction *)array_get(&func->instructions, j);
                if (prev_instr && prev_instr->opcode == IR_LABEL && 
                    prev_instr->label && strcmp(prev_instr->label, instr->label) == 0)
                {
                    hashtable_put(loop_labels, instr->label, (void *)1);
                    break;
                }
            }
        }
    }

    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (!instr)
            continue;
            
        if ((instr->opcode >= IR_LT && instr->opcode <= IR_GE) && instr->arg1)
        {
            if (i + 1 < func->instructions.size)
            {
                IRInstruction *next_instr = (IRInstruction *)array_get(&func->instructions, i + 1);
                if (next_instr && (next_instr->opcode == IR_JUMP_IF_FALSE || 
                                   next_instr->opcode == IR_JUMP_IF))
                {
                    if (next_instr->label && hashtable_contains(loop_labels, next_instr->label))
                    {
                        if (instr->arg1->type == IR_OP_VAR)
                        {
                            hashtable_remove(cp_state->constants, instr->arg1->data.var_name);
                            if (debug_enabled)
                            {
                                printf("[DEBUG] Removing constant for %s (used in loop condition)\n", 
                                       instr->arg1->data.var_name);
                            }
                        }
                        if (instr->arg2 && instr->arg2->type == IR_OP_VAR)
                        {
                            hashtable_remove(cp_state->constants, instr->arg2->data.var_name);
                            if (debug_enabled)
                            {
                                printf("[DEBUG] Removing constant for %s (used in loop condition)\n", 
                                       instr->arg2->data.var_name);
                            }
                        }
                    }
                }
            }
        }
        
        if (instr->opcode == IR_LABEL && instr->label && hashtable_contains(loop_labels, instr->label))
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Entering loop at label %s, invalidating constants\n", instr->label);
            }
            hashtable_destroy(cp_state->constants);
            cp_state->constants = hashtable_create(16);
        }

        if ((instr->opcode >= IR_ADD && instr->opcode <= IR_OR) && 
            instr->arg1 && instr->arg2 && instr->result)
        {
            IROperand *folded = fold_binary_op(instr->opcode, instr->arg1, instr->arg2, instr->result->data_type);
            if (folded)
            {
                IROperand *old_arg1 = instr->arg1;
                IROperand *old_arg2 = instr->arg2;
                instr->opcode = IR_MOVE;
                instr->arg1 = folded;
                instr->arg2 = NULL;
                ir_operand_destroy(old_arg1);
                ir_operand_destroy(old_arg2);
                changed = true;

                if (debug_enabled)
                {
                    printf("[DEBUG] Folded binary operation at instruction %zu\n", i);
                }
            }
        }

        if ((instr->opcode == IR_NEG || instr->opcode == IR_NOT) && 
            instr->arg1 && instr->result)
        {
            IROperand *folded = fold_unary_op(instr->opcode, instr->arg1, instr->result->data_type);
            if (folded)
            {
                IROperand *old_arg1 = instr->arg1;
                instr->opcode = IR_MOVE;
                instr->arg1 = folded;
                instr->arg2 = NULL;
                ir_operand_destroy(old_arg1);
                changed = true;

                if (debug_enabled)
                {
                    printf("[DEBUG] Folded unary operation at instruction %zu\n", i);
                }
            }
        }

        if (instr->opcode == IR_JUMP && instr->label && hashtable_contains(loop_labels, instr->label))
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Detected backward jump to loop %s, invalidating constants\n", instr->label);
            }
            hashtable_destroy(cp_state->constants);
            cp_state->constants = hashtable_create(16);
        }

        if (instr->opcode == IR_MOVE && instr->result && instr->arg1)
        {
            if (is_constant(instr->arg1))
            {
                if (instr->result->type == IR_OP_VAR)
                {
                    IROperand *old_const = (IROperand *)hashtable_get(cp_state->constants, instr->result->data.var_name);
                    if (old_const)
                    {
                        ir_operand_destroy(old_const);
                    }
                    
                    IROperand *const_copy = NULL;
                    if (instr->arg1->is_float_const)
                    {
                        const_copy = ir_operand_float_const(get_const_float_value(instr->arg1));
                    }
                    else
                    {
                        const_copy = ir_operand_const(get_const_value(instr->arg1));
                    }
                    hashtable_put(cp_state->constants, instr->result->data.var_name, const_copy);
                }
                else if (instr->result->type == IR_OP_TEMP)
                {
                    char temp_key[32];
                    snprintf(temp_key, sizeof(temp_key), "t%d", instr->result->data.temp_id);
                    
                    IROperand *old_const = (IROperand *)hashtable_get(cp_state->constants, temp_key);
                    if (old_const)
                    {
                        ir_operand_destroy(old_const);
                    }
                    
                    IROperand *const_copy = NULL;
                    if (instr->arg1->is_float_const)
                    {
                        const_copy = ir_operand_float_const(get_const_float_value(instr->arg1));
                    }
                    else
                    {
                        const_copy = ir_operand_const(get_const_value(instr->arg1));
                    }
                    hashtable_put(cp_state->constants, temp_key, const_copy);
                }
            }
            else if (instr->result->type == IR_OP_VAR)
            {
                IROperand *old_const = (IROperand *)hashtable_get(cp_state->constants, instr->result->data.var_name);
                if (old_const)
                {
                    ir_operand_destroy(old_const);
                }
                hashtable_remove(cp_state->constants, instr->result->data.var_name);
            }
            else if (instr->result->type == IR_OP_TEMP)
            {
                char temp_key[32];
                snprintf(temp_key, sizeof(temp_key), "t%d", instr->result->data.temp_id);
                IROperand *old_const = (IROperand *)hashtable_get(cp_state->constants, temp_key);
                if (old_const)
                {
                    ir_operand_destroy(old_const);
                }
                hashtable_remove(cp_state->constants, temp_key);
            }
        }
        
        if ((instr->opcode >= IR_ADD && instr->opcode <= IR_OR) && instr->result)
        {
            if (instr->result->type == IR_OP_VAR)
            {
                IROperand *old_const = (IROperand *)hashtable_get(cp_state->constants, instr->result->data.var_name);
                if (old_const)
                {
                    ir_operand_destroy(old_const);
                }
                hashtable_remove(cp_state->constants, instr->result->data.var_name);
            }
            else if (instr->result->type == IR_OP_TEMP)
            {
                char temp_key[32];
                snprintf(temp_key, sizeof(temp_key), "t%d", instr->result->data.temp_id);
                IROperand *old_const = (IROperand *)hashtable_get(cp_state->constants, temp_key);
                if (old_const)
                {
                    ir_operand_destroy(old_const);
                }
                hashtable_remove(cp_state->constants, temp_key);
            }
        }

        if (instr->arg1)
        {
            IROperand *const_val = NULL;
            char temp_key[32];
            bool is_var = (instr->arg1->type == IR_OP_VAR);
            bool is_temp = (instr->arg1->type == IR_OP_TEMP);
            
            if (is_var)
            {
                const_val = (IROperand *)hashtable_get(cp_state->constants, instr->arg1->data.var_name);
            }
            else if (is_temp)
            {
                snprintf(temp_key, sizeof(temp_key), "t%d", instr->arg1->data.temp_id);
                const_val = (IROperand *)hashtable_get(cp_state->constants, temp_key);
            }
            
            if (const_val && is_constant(const_val))
            {
                IROperand *new_const = NULL;
                if (const_val->is_float_const)
                {
                    new_const = ir_operand_float_const(get_const_float_value(const_val));
                }
                else
                {
                    new_const = ir_operand_const(get_const_value(const_val));
                }
                if (instr->arg1->type == IR_OP_CONST || instr->arg1->type == IR_OP_STRING_CONST)
                {
                    ir_operand_destroy(instr->arg1);
                }
                instr->arg1 = new_const;
                changed = true;

                if (debug_enabled)
                {
                    if (is_var)
                    {
                        printf("[DEBUG] Propagated constant for variable at instruction %zu\n", i);
                    }
                    else if (is_temp)
                    {
                        printf("[DEBUG] Propagated constant for temp at instruction %zu\n", i);
                    }
                }
            }
        }

        if (instr->arg2)
        {
            IROperand *const_val = NULL;
            char temp_key[32];
            
            if (instr->arg2->type == IR_OP_VAR)
            {
                const_val = (IROperand *)hashtable_get(cp_state->constants, instr->arg2->data.var_name);
            }
            else if (instr->arg2->type == IR_OP_TEMP)
            {
                snprintf(temp_key, sizeof(temp_key), "t%d", instr->arg2->data.temp_id);
                const_val = (IROperand *)hashtable_get(cp_state->constants, temp_key);
            }
            
            if (const_val && is_constant(const_val))
            {
                IROperand *new_const = NULL;
                if (const_val->is_float_const)
                {
                    new_const = ir_operand_float_const(get_const_float_value(const_val));
                }
                else
                {
                    new_const = ir_operand_const(get_const_value(const_val));
                }
                if (instr->arg2->type == IR_OP_CONST || instr->arg2->type == IR_OP_STRING_CONST)
                {
                    ir_operand_destroy(instr->arg2);
                }
                instr->arg2 = new_const;
                changed = true;

                if (debug_enabled)
                {
                    printf("[DEBUG] Propagated constant for arg2 at instruction %zu\n", i);
                }
            }
        }
    }

    hashtable_destroy(loop_labels);
    cp_state_destroy(cp_state);
    return changed;
}

bool optimization_constant_folding(IRProgram *program)
{
    if (!program)
        return false;

    bool changed = false;
    for (size_t i = 0; i < program->functions.size; i++)
    {
        IRFunction *func = (IRFunction *)array_get(&program->functions, i);
        if (func)
        {
            if (optimize_function_constant_folding(func))
            {
                changed = true;
            }
        }
    }

    return changed;
}

