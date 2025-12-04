#include "backend/codegen/codegenCore.h"
#include "backend/codegen/codegenIH.h"
#include "backend/ir/irTypes.h"
#include "backend/ir/irOps.h"
#include "common/common.h"
#include <stdio.h>
#include <string.h>

extern bool debug_enabled;

static void analyze_temp_usage(IRFunction *func, HashTable *temp_use_count, HashTable *temp_defining_instr)
{
    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (!instr)
            continue;
        
        if (instr->result && instr->result->type == IR_OP_TEMP)
        {
            char key[32];
            snprintf(key, sizeof(key), "t%d", instr->result->data.temp_id);
            hashtable_put(temp_defining_instr, key, (void *)(intptr_t)i);
            
            if (!hashtable_contains(temp_use_count, key))
            {
                hashtable_put(temp_use_count, key, (void *)0);
            }
        }
        
        if (instr->arg1 && instr->arg1->type == IR_OP_TEMP)
        {
            char key[32];
            snprintf(key, sizeof(key), "t%d", instr->arg1->data.temp_id);
            intptr_t count = (intptr_t)hashtable_get(temp_use_count, key);
            hashtable_put(temp_use_count, key, (void *)(count + 1));
        }
        
        if (instr->arg2 && instr->arg2->type == IR_OP_TEMP)
        {
            char key[32];
            snprintf(key, sizeof(key), "t%d", instr->arg2->data.temp_id);
            intptr_t count = (intptr_t)hashtable_get(temp_use_count, key);
            hashtable_put(temp_use_count, key, (void *)(count + 1));
        }
        
        if (instr->args)
        {
            for (size_t j = 0; j < instr->args->size; j++)
            {
                IROperand *arg = (IROperand *)array_get(instr->args, j);
                if (arg && arg->type == IR_OP_TEMP)
                {
                    char key[32];
                    snprintf(key, sizeof(key), "t%d", arg->data.temp_id);
                    intptr_t count = (intptr_t)hashtable_get(temp_use_count, key);
                    hashtable_put(temp_use_count, key, (void *)(count + 1));
                }
            }
        }
    }
}

static bool can_inline_temp_to_var(IRFunction *func, int temp_id, HashTable *temp_use_count, size_t *move_instr_idx)
{
    char key[32];
    snprintf(key, sizeof(key), "t%d", temp_id);
    
    intptr_t use_count = (intptr_t)hashtable_get(temp_use_count, key);
    if (use_count != 1)
        return false;
    
    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (!instr)
            continue;

        if (instr->opcode == IR_MOVE && 
            instr->arg1 && instr->arg1->type == IR_OP_TEMP && 
            instr->arg1->data.temp_id == temp_id &&
            instr->result && instr->result->type == IR_OP_VAR)
        {
            *move_instr_idx = i;
            return true;
        }
    }
    
    return false;
}

static bool can_inline_temp_direct(IRFunction *func, int temp_id, HashTable *temp_use_count, size_t *use_instr_idx)
{
    char key[32];
    snprintf(key, sizeof(key), "t%d", temp_id);
    
    intptr_t use_count = (intptr_t)hashtable_get(temp_use_count, key);
    if (use_count != 1)
        return false;
    
    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (!instr)
            continue;
        
        bool uses_temp = false;
        if (instr->arg1 && instr->arg1->type == IR_OP_TEMP && instr->arg1->data.temp_id == temp_id)
            uses_temp = true;
        if (instr->arg2 && instr->arg2->type == IR_OP_TEMP && instr->arg2->data.temp_id == temp_id)
            uses_temp = true;
        if (instr->args)
        {
            for (size_t j = 0; j < instr->args->size; j++)
            {
                IROperand *arg = (IROperand *)array_get(instr->args, j);
                if (arg && arg->type == IR_OP_TEMP && arg->data.temp_id == temp_id)
                {
                    uses_temp = true;
                    break;
                }
            }
        }
        
        if (uses_temp && (instr->opcode == IR_PRINT || instr->opcode == IR_CALL || 
                          instr->opcode == IR_RETURN || instr->opcode == IR_ARRAY_STORE))
        {
            *use_instr_idx = i;
            return true;
        }
    }
    
    return false;
}

void codegen_peephole_optimize_function(IRFunction *func)
{
    HashTable *temp_use_count = hashtable_create(32);
    HashTable *temp_defining_instr = hashtable_create(32);
    analyze_temp_usage(func, temp_use_count, temp_defining_instr);
    HashTable *skip_instrs = hashtable_create(32);
    
    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (!instr || instr->opcode != IR_CALL || !instr->result || instr->result->type != IR_OP_TEMP)
            continue;
        
        int temp_id = instr->result->data.temp_id;
        size_t move_instr_idx;
        size_t use_instr_idx;
        
        if (can_inline_temp_to_var(func, temp_id, temp_use_count, &move_instr_idx))
        {
            IRInstruction *move_instr = (IRInstruction *)array_get(&func->instructions, move_instr_idx);
            if (move_instr && move_instr->opcode == IR_MOVE && move_instr->result && move_instr->result->type == IR_OP_VAR)
            {
                IROperand *old_result = instr->result;
                instr->result = move_instr->result;
                char key[32];
                snprintf(key, sizeof(key), "skip_%zu", move_instr_idx);
                hashtable_put(skip_instrs, key, (void *)1);
            }
        }
        else if (can_inline_temp_direct(func, temp_id, temp_use_count, &use_instr_idx))
        {
            IRInstruction *use_instr = (IRInstruction *)array_get(&func->instructions, use_instr_idx);
            if (use_instr)
            {
                if (use_instr->arg1 && use_instr->arg1->type == IR_OP_TEMP && use_instr->arg1->data.temp_id == temp_id)
                {
                    char key[32];
                    snprintf(key, sizeof(key), "inline_call_%zu", use_instr_idx);
                    hashtable_put(skip_instrs, key, (void *)(intptr_t)i);
                }
            }
        }
        char key[32];
        snprintf(key, sizeof(key), "t%d", temp_id);
        intptr_t use_count = (intptr_t)hashtable_get(temp_use_count, key);
        if (use_count == 0)
        {
            if (instr->result)
            {
                instr->result = NULL;
            }
        }
    }
    
    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (!instr)
            continue;
        
        if (instr->opcode == IR_NE && 
            instr->result && instr->result->type == IR_OP_TEMP &&
            instr->arg2 && instr->arg2->type == IR_OP_CONST && instr->arg2->data.const_value == 0)
        {
            int temp_id = instr->result->data.temp_id;
            
            if (i + 1 < func->instructions.size)
            {
                IRInstruction *next = (IRInstruction *)array_get(&func->instructions, i + 1);
                if (next && next->opcode == IR_JUMP_IF_FALSE &&
                    next->arg1 && next->arg1->type == IR_OP_TEMP &&
                    next->arg1->data.temp_id == temp_id)
                {
                    char key[32];
                    snprintf(key, sizeof(key), "t%d", temp_id);
                    intptr_t use_count = (intptr_t)hashtable_get(temp_use_count, key);
                    if (use_count == 1)
                    {
                        IROperand *old_arg1 = next->arg1;
                        next->arg1 = instr->arg1;
                        snprintf(key, sizeof(key), "skip_%zu", i);
                        hashtable_put(skip_instrs, key, (void *)1);
                    }
                }
            }
        }
    }
    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (!instr || instr->opcode != IR_CALL || !instr->result || instr->result->type != IR_OP_TEMP)
            continue;
        
        int temp_id = instr->result->data.temp_id;
        for (size_t j = i + 1; j < func->instructions.size; j++)
        {
            IRInstruction *next_instr = (IRInstruction *)array_get(&func->instructions, j);
            if (!next_instr)
                continue;
            if (next_instr->opcode == IR_PARAM &&
                next_instr->arg1 && next_instr->arg1->type == IR_OP_TEMP &&
                next_instr->arg1->data.temp_id == temp_id)
            {
                char key[32];
                snprintf(key, sizeof(key), "t%d", temp_id);
                intptr_t use_count = (intptr_t)hashtable_get(temp_use_count, key);
                if (use_count == 1)
                {
                }
            }
            
            if (next_instr->opcode == IR_CALL)
                break;
        }
    }
    DynamicArray new_instructions;
    array_init(&new_instructions, func->instructions.size);
    
    for (size_t i = 0; i < func->instructions.size; i++)
    {
        char key[32];
        snprintf(key, sizeof(key), "skip_%zu", i);
        if (!hashtable_contains(skip_instrs, key))
        {
            IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
            array_push(&new_instructions, instr);
        }
        else
        {
        }
    }
    
    safe_free(func->instructions.data);
    func->instructions.data = new_instructions.data;
    func->instructions.size = new_instructions.size;
    func->instructions.capacity = new_instructions.capacity;
    new_instructions.data = NULL;
    new_instructions.size = 0;
    new_instructions.capacity = 0;
    
    hashtable_destroy(temp_use_count);
    hashtable_destroy(temp_defining_instr);
    hashtable_destroy(skip_instrs);
}

void codegen_peephole_optimize_program(IRProgram *program)
{
    if (!program)
        return;
    
    for (size_t i = 0; i < program->functions.size; i++)
    {
        IRFunction *func = (IRFunction *)array_get(&program->functions, i);
        if (func)
        {
            codegen_peephole_optimize_function(func);
        }
    }
}

