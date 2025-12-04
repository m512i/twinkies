#include "optimizations/optimizer.h"
#include "backend/ir/irCore.h"
#include "backend/ir/irTypes.h"
#include "backend/ir/irOps.h"
#include "common/common.h"
#include "common/flags.h"
#include <stdio.h>
#include <string.h>

extern bool debug_enabled;

static void get_operand_key(IROperand *op, char *key, size_t key_size)
{
    if (!op)
    {
        key[0] = '\0';
        return;
    }
    
    if (op->type == IR_OP_VAR)
    {
        snprintf(key, key_size, "v:%s", op->data.var_name);
    }
    else if (op->type == IR_OP_TEMP)
    {
        snprintf(key, key_size, "t:%d", op->data.temp_id);
    }
    else
    {
        key[0] = '\0';
    }
}

static bool is_simple_operand(IROperand *op)
{
    return op && (op->type == IR_OP_VAR || op->type == IR_OP_TEMP);
}

static bool optimize_function_copy_propagation(IRFunction *func)
{
    bool changed = false;
    HashTable *copy_map = hashtable_create(32);
    
    HashTable *redefined = hashtable_create(32);
    
    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (!instr)
            continue;
        
        if (instr->opcode == IR_MOVE && instr->result && instr->arg1)
        {
            if (is_simple_operand(instr->result) && is_simple_operand(instr->arg1))
            {
                char result_key[64];
                get_operand_key(instr->result, result_key, sizeof(result_key));
                char arg1_key[64];
                get_operand_key(instr->arg1, arg1_key, sizeof(arg1_key));
                
                IROperand *source = (IROperand *)hashtable_get(copy_map, arg1_key);
                if (source)
                {
                    IROperand *source_copy = NULL;
                    if (source->type == IR_OP_VAR)
                    {
                        source_copy = ir_operand_var(source->data.var_name);
                        source_copy->data_type = source->data_type;
                    }
                    else if (source->type == IR_OP_TEMP)
                    {
                        source_copy = ir_operand_temp(source->data.temp_id);
                        source_copy->data_type = source->data_type;
                    }
                    
                    if (source_copy)
                    {
                        IROperand *old = (IROperand *)hashtable_get(copy_map, result_key);
                        if (old)
                        {
                            ir_operand_destroy(old);
                        }
                        hashtable_put(copy_map, result_key, source_copy);
                    }
                }
                else
                {
                    IROperand *arg1_copy = NULL;
                    if (instr->arg1->type == IR_OP_VAR)
                    {
                        arg1_copy = ir_operand_var(instr->arg1->data.var_name);
                        arg1_copy->data_type = instr->arg1->data_type;
                    }
                    else if (instr->arg1->type == IR_OP_TEMP)
                    {
                        arg1_copy = ir_operand_temp(instr->arg1->data.temp_id);
                        arg1_copy->data_type = instr->arg1->data_type;
                    }
                    
                    if (arg1_copy)
                    {
                        IROperand *old = (IROperand *)hashtable_get(copy_map, result_key);
                        if (old)
                        {
                            ir_operand_destroy(old);
                        }
                        hashtable_put(copy_map, result_key, arg1_copy);
                    }
                }
            }
            else
            {
                if (is_simple_operand(instr->result))
                {
                    char result_key[64];
                    get_operand_key(instr->result, result_key, sizeof(result_key));
                    IROperand *old = (IROperand *)hashtable_get(copy_map, result_key);
                    if (old)
                    {
                        ir_operand_destroy(old);
                        hashtable_remove(copy_map, result_key);
                    }
                    hashtable_put(redefined, result_key, (void *)1);
                }
            }
        }
        else if (instr->result && is_simple_operand(instr->result))
        {
            char result_key[64];
            get_operand_key(instr->result, result_key, sizeof(result_key));
            IROperand *old = (IROperand *)hashtable_get(copy_map, result_key);
            if (old)
            {
                ir_operand_destroy(old);
                hashtable_remove(copy_map, result_key);
            }
            hashtable_put(redefined, result_key, (void *)1);
        }
    }
    
    for (size_t i = 0; i < func->instructions.size; i++)
    {
        IRInstruction *instr = (IRInstruction *)array_get(&func->instructions, i);
        if (!instr)
            continue;
        
        if (instr->arg1 && is_simple_operand(instr->arg1))
        {
            char arg1_key[64];
            get_operand_key(instr->arg1, arg1_key, sizeof(arg1_key));
            
            if (!hashtable_contains(redefined, arg1_key))
            {
                IROperand *source = (IROperand *)hashtable_get(copy_map, arg1_key);
                if (source)
                {
                    IROperand *new_arg = NULL;
                    if (source->type == IR_OP_VAR)
                    {
                        new_arg = ir_operand_var(source->data.var_name);
                        new_arg->data_type = source->data_type;
                    }
                    else if (source->type == IR_OP_TEMP)
                    {
                        new_arg = ir_operand_temp(source->data.temp_id);
                        new_arg->data_type = source->data_type;
                    }
                    
                    if (new_arg)
                    {
                        if (instr->arg1->type == IR_OP_CONST || instr->arg1->type == IR_OP_STRING_CONST)
                        {
                            ir_operand_destroy(instr->arg1);
                        }
                        instr->arg1 = new_arg;
                        changed = true;
                        
                        if (debug_enabled)
                        {
                            printf("[DEBUG] Copy propagation: Replaced arg1 at instruction %zu\n", i);
                        }
                    }
                }
            }
        }
        
        if (instr->arg2 && is_simple_operand(instr->arg2))
        {
            char arg2_key[64];
            get_operand_key(instr->arg2, arg2_key, sizeof(arg2_key));
            
            if (!hashtable_contains(redefined, arg2_key))
            {
                IROperand *source = (IROperand *)hashtable_get(copy_map, arg2_key);
                if (source)
                {
                    IROperand *new_arg = NULL;
                    if (source->type == IR_OP_VAR)
                    {
                        new_arg = ir_operand_var(source->data.var_name);
                        new_arg->data_type = source->data_type;
                    }
                    else if (source->type == IR_OP_TEMP)
                    {
                        new_arg = ir_operand_temp(source->data.temp_id);
                        new_arg->data_type = source->data_type;
                    }
                    
                    if (new_arg)
                    {
                        if (instr->arg2->type == IR_OP_CONST || instr->arg2->type == IR_OP_STRING_CONST)
                        {
                            ir_operand_destroy(instr->arg2);
                        }
                        instr->arg2 = new_arg;
                        changed = true;
                        
                        if (debug_enabled)
                        {
                            printf("[DEBUG] Copy propagation: Replaced arg2 at instruction %zu\n", i);
                        }
                    }
                }
            }
        }
        
        if (instr->args)
        {
            for (size_t j = 0; j < instr->args->size; j++)
            {
                IROperand *arg = (IROperand *)array_get(instr->args, j);
                if (arg && is_simple_operand(arg))
                {
                    char arg_key[64];
                    get_operand_key(arg, arg_key, sizeof(arg_key));
                    
                    if (!hashtable_contains(redefined, arg_key))
                    {
                        IROperand *source = (IROperand *)hashtable_get(copy_map, arg_key);
                        if (source)
                        {
                            IROperand *new_arg = NULL;
                            if (source->type == IR_OP_VAR)
                            {
                                new_arg = ir_operand_var(source->data.var_name);
                                new_arg->data_type = source->data_type;
                            }
                            else if (source->type == IR_OP_TEMP)
                            {
                                new_arg = ir_operand_temp(source->data.temp_id);
                                new_arg->data_type = source->data_type;
                            }
                            
                            if (new_arg)
                            {
                                if (arg->type == IR_OP_CONST || arg->type == IR_OP_STRING_CONST)
                                {
                                    ir_operand_destroy(arg);
                                }
                                array_set(instr->args, j, new_arg);
                                changed = true;
                                
                                if (debug_enabled)
                                {
                                    printf("[DEBUG] Copy propagation: Replaced arg in args array at instruction %zu, index %zu\n", i, j);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    
    for (size_t i = 0; i < copy_map->capacity; i++)
    {
        HashTableEntry *entry = copy_map->buckets[i];
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
    hashtable_destroy(copy_map);
    hashtable_destroy(redefined);
    
    return changed;
}

bool optimization_copy_propagation(IRProgram *program)
{
    if (!program)
        return false;
        
    bool changed = false;
    
    for (size_t i = 0; i < program->functions.size; i++)
    {
        IRFunction *func = (IRFunction *)array_get(&program->functions, i);
        if (!func)
            continue;
            
        if (optimize_function_copy_propagation(func))
        {
            changed = true;
        }
    }
    
    return changed;
}

