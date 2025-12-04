#include "optimizations/optimizer.h"
#include "backend/ir/irCore.h"
#include "backend/ir/irOps.h"
#include "backend/ir/irinstructions.h"
#include "common/common.h"

extern bool debug_enabled;

static void mark_reachable_labels(IRFunction *func, HashTable *reachable_labels)
{
    if (func->instructions.size == 0)
        return;

    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (!instr)
            continue;

        if (instr->opcode == IR_JUMP && instr->label)
        {
            hashtable_put(reachable_labels, instr->label, (void *)1);
        }
        else if (instr->opcode == IR_JUMP_IF && instr->label)
        {
            hashtable_put(reachable_labels, instr->label, (void *)1);
        }
        else if (instr->opcode == IR_JUMP_IF_FALSE && instr->label)
        {
            hashtable_put(reachable_labels, instr->label, (void *)1);
        }
        else if (instr->opcode == IR_LABEL && instr->label)
        {
            if (i == 0)
            {
                hashtable_put(reachable_labels, instr->label, (void *)1);
            }
        }
    }

    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (instr && instr->opcode == IR_LABEL && instr->label)
        {
            hashtable_put(reachable_labels, instr->label, (void *)1);
            break;
        }
        if (instr && instr->opcode != IR_LABEL)
        {
            break;
        }
    }
}

static void analyze_uses(IRFunction *func, HashTable *used_vars, HashTable *used_temps)
{
    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (!instr)
            continue;

        if (instr->arg1)
        {
            if (instr->arg1->type == IR_OP_VAR)
            {
                hashtable_put(used_vars, instr->arg1->data.var_name, (void *)1);
            }
            else if (instr->arg1->type == IR_OP_TEMP)
            {
                char temp_key[32];
                snprintf(temp_key, sizeof(temp_key), "t%d", instr->arg1->data.temp_id);
                hashtable_put(used_temps, temp_key, (void *)1);
            }
        }

        if (instr->arg2)
        {
            if (instr->arg2->type == IR_OP_VAR)
            {
                hashtable_put(used_vars, instr->arg2->data.var_name, (void *)1);
            }
            else if (instr->arg2->type == IR_OP_TEMP)
            {
                char temp_key[32];
                snprintf(temp_key, sizeof(temp_key), "t%d", instr->arg2->data.temp_id);
                hashtable_put(used_temps, temp_key, (void *)1);
            }
        }

        if (instr->opcode == IR_PRINT || 
            instr->opcode == IR_PRINT_MULTIPLE ||
            instr->opcode == IR_CALL ||
            instr->opcode == IR_RETURN ||
            instr->opcode == IR_ARRAY_STORE ||
            instr->opcode == IR_INLINE_ASM)
        {
            if (instr->result && instr->result->type == IR_OP_VAR)
            {
                hashtable_put(used_vars, instr->result->data.var_name, (void *)1);
            }
            
            if (instr->opcode == IR_PRINT_MULTIPLE && instr->args)
            {
                for (size_t j = 0; j < instr->args->size; j++)
                {
                    IROperand *arg = (IROperand *)array_get(instr->args, j);
                    if (arg && arg->type == IR_OP_VAR)
                    {
                        hashtable_put(used_vars, arg->data.var_name, (void *)1);
                    }
                    else if (arg && arg->type == IR_OP_TEMP)
                    {
                        char temp_key[32];
                        snprintf(temp_key, sizeof(temp_key), "t%d", arg->data.temp_id);
                        hashtable_put(used_temps, temp_key, (void *)1);
                    }
                }
            }
        }
        
        if (instr->result && instr->result->type == IR_OP_VAR)
        {
            hashtable_put(used_vars, instr->result->data.var_name, (void *)1);
        }
    }
}

static bool eliminate_dead_code(IRFunction *func)
{
    bool changed = false;
    HashTable *reachable_labels = hashtable_create(16);
    HashTable *used_vars = hashtable_create(32);
    HashTable *used_temps = hashtable_create(32);

    mark_reachable_labels(func, reachable_labels);

    analyze_uses(func, used_vars, used_temps);

    DynamicArray new_instructions;
    size_t initial_capacity = func->instructions.size > 0 ? func->instructions.size : 16;
    array_init(&new_instructions, initial_capacity);

    bool in_unreachable_block = false;
    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (!instr)
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Dead code elimination: Found NULL instruction at index %zu\n", i);
                fflush(stdout);
            }
            continue;
        }

        if (instr->opcode == IR_LABEL && instr->label)
        {
            if (!hashtable_contains(reachable_labels, instr->label))
            {
                in_unreachable_block = true;
                if (debug_enabled)
                {
                    printf("[DEBUG] Marking label %s as unreachable\n", instr->label);
                }
            }
            else
            {
                in_unreachable_block = false;
            }
        }

        if (instr->opcode == IR_JUMP || 
            instr->opcode == IR_JUMP_IF || 
            instr->opcode == IR_JUMP_IF_FALSE ||
            instr->opcode == IR_RETURN)
        {
            if (instr->opcode == IR_JUMP && instr->label && 
                hashtable_contains(reachable_labels, instr->label))
            {
                in_unreachable_block = false;
            }
        }

        if (in_unreachable_block && instr->opcode != IR_LABEL)
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Removing unreachable instruction at %zu\n", i);
                fflush(stdout);
            }
            ir_instruction_destroy(instr);
            changed = true;
            continue;
        }

        bool is_dead_assignment = false;
        if (instr->opcode == IR_MOVE && instr->result)
        {
            if (instr->result->type == IR_OP_VAR)
            {
                if (!hashtable_contains(used_vars, instr->result->data.var_name))
                {
                    if (instr->arg1)
                    {
                        is_dead_assignment = true;
                    }
                }
            }
            else if (instr->result->type == IR_OP_TEMP)
            {
                char temp_key[32];
                snprintf(temp_key, sizeof(temp_key), "t%d", instr->result->data.temp_id);
                if (!hashtable_contains(used_temps, temp_key))
                {
                    if (instr->arg1)
                    {
                        is_dead_assignment = true;
                    }
                }
            }
        }
        
        if ((instr->opcode >= IR_ADD && instr->opcode <= IR_OR) && instr->result)
        {
            if (instr->result->type == IR_OP_VAR)
            {
                if (!hashtable_contains(used_vars, instr->result->data.var_name))
                {
                    is_dead_assignment = true;
                }
            }
            else if (instr->result->type == IR_OP_TEMP)
            {
                char temp_key[32];
                snprintf(temp_key, sizeof(temp_key), "t%d", instr->result->data.temp_id);
                if (!hashtable_contains(used_temps, temp_key))
                {
                    is_dead_assignment = true;
                }
            }
        }

        if (instr->opcode == IR_NOP)
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Removing NOP instruction at %zu\n", i);
                fflush(stdout);
            }
            ir_instruction_destroy(instr);
            changed = true;
            continue;
        }

        if (instr->opcode == IR_VAR_DECL && instr->result && instr->result->type == IR_OP_VAR)
        {
            if (!hashtable_contains(used_vars, instr->result->data.var_name))
            {
                if (debug_enabled)
                {
                    printf("[DEBUG] Removing unused variable declaration: %s\n", instr->result->data.var_name);
                    fflush(stdout);
                }
                ir_instruction_destroy(instr);
                changed = true;
                continue;
            }
        }

        if (is_dead_assignment)
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Removing dead assignment at instruction %zu, opcode=%d\n", i, instr->opcode);
                if (instr->result)
                {
                    printf("[DEBUG]   Result type=%d\n", instr->result->type);
                }
                if (instr->arg1)
                {
                    printf("[DEBUG]   Arg1 type=%d\n", instr->arg1->type);
                }
                fflush(stdout);
            }
            
            ir_instruction_destroy(instr);
            changed = true;
            if (debug_enabled)
            {
                printf("[DEBUG] Dead assignment destroyed, continuing loop\n");
                fflush(stdout);
            }
            continue;
        }

        if (debug_enabled && i < 5)
        {
            printf("[DEBUG] Keeping instruction %zu, opcode=%d\n", i, instr->opcode);
            fflush(stdout);
        }
        array_push(&new_instructions, instr);
    }

    if (changed)
    {
        if (debug_enabled)
        {
            printf("[DEBUG] Dead code elimination: Replacing instruction array, old size=%zu, new size=%zu\n", 
                   func->instructions.size, new_instructions.size);
            fflush(stdout);
        }
        safe_free(func->instructions.data);
        func->instructions.data = new_instructions.data;
        func->instructions.size = new_instructions.size;
        func->instructions.capacity = new_instructions.capacity;
        new_instructions.data = NULL;
        new_instructions.size = 0;
        new_instructions.capacity = 0;
    }
    else
    {
        array_free(&new_instructions);
    }

    hashtable_destroy(reachable_labels);
    hashtable_destroy(used_vars);
    hashtable_destroy(used_temps);

    if (debug_enabled)
    {
        printf("[DEBUG] Dead code elimination: Completed for function\n");
        fflush(stdout);
    }

    return changed;
}

bool optimization_dead_code_elimination(IRProgram *program)
{
    if (!program)
        return false;

    bool changed = false;
    for (size_t i = 0; i < program->functions.size; i++)
    {
        IRFunction *func = (IRFunction *)array_get(&program->functions, i);
        if (func)
        {
            if (eliminate_dead_code(func))
            {
                changed = true;
            }
        }
    }

    return changed;
}

