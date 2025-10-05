#include "backend/ir/ir.h"
#include "analysis/semantic/semantic.h"
#include "backend/ir/iroperands.h"
#include "backend/ir/irinstructions.h"
#include "common/common.h"
#include "modules/modules.h"

extern bool debug_enabled;

IRFunction *ir_generate_function_impl(Function *func, SemanticAnalyzer *analyzer);
void ir_generate_statement_impl(IRFunction *ir_func, Stmt *stmt, SemanticAnalyzer *analyzer);


IRProgram *ir_generate_with_modules_impl(Program *ast_program, SemanticAnalyzer *analyzer, void *module_manager)
{
    IRProgram *ir_program = ir_program_create();

    if (debug_enabled)
    {
        printf("[DEBUG] ir_generate_with_modules: Starting IR generation\n");
        printf("[DEBUG] ir_generate_with_modules: Main program has %zu functions\n", ast_program->functions.size);
    }

    for (size_t i = 0; i < ast_program->functions.size; i++)
    {
        Function *func = (Function *)array_get(&ast_program->functions, i);
        if (debug_enabled)
        {
            printf("[DEBUG] ir_generate_with_modules: Generating IR for main program function: %s\n", func->name);
        }
        IRFunction *ir_func = ir_generate_function_impl(func, analyzer);
        ir_program_add_function(ir_program, ir_func);
    }

    if (module_manager)
    {
        ModuleManager *manager = (ModuleManager *)module_manager;
        if (debug_enabled)
        {
            printf("[DEBUG] ir_generate_with_modules: Processing %zu modules\n", manager->modules.size);
        }

        for (size_t i = 0; i < manager->modules.size; i++)
        {
            Module *module = (Module *)array_get(&manager->modules, i);
            if (module->ast)
            {
                if (debug_enabled)
                {
                    printf("[DEBUG] ir_generate_with_modules: Processing module %s with %zu functions\n",
                           module->name, module->ast->functions.size);
                }

                for (size_t j = 0; j < module->ast->functions.size; j++)
                {
                    Function *func = (Function *)array_get(&module->ast->functions, j);
                    if (func->body)
                    {
                        if (debug_enabled)
                        {
                            printf("[DEBUG] ir_generate_with_modules: Generating IR for module function: %s\n", func->name);
                        }
                        IRFunction *ir_func = ir_generate_function_impl(func, analyzer);
                        ir_program_add_function(ir_program, ir_func);
                    }
                }
            }
        }
    }

    if (debug_enabled)
    {
        printf("[DEBUG] ir_generate_with_modules: IR generation completed, program has %zu functions\n", ir_program->functions.size);
    }

    return ir_program;
}

IRProgram *ir_generate_impl(Program *ast_program, SemanticAnalyzer *analyzer)
{
    IRProgram *ir_program = ir_program_create();

    if (debug_enabled)
    {
        printf("[DEBUG] ir_generate: Starting IR generation\n");
        printf("[DEBUG] ir_generate: Main program has %zu functions\n", ast_program->functions.size);
    }

    for (size_t i = 0; i < ast_program->functions.size; i++)
    {
        Function *func = (Function *)array_get(&ast_program->functions, i);
        if (debug_enabled)
        {
            printf("[DEBUG] ir_generate: Generating IR for main program function: %s\n", func->name);
        }
        IRFunction *ir_func = ir_generate_function_impl(func, analyzer);
        ir_program_add_function(ir_program, ir_func);
    }

    /* Generate IR for module functions from the module manager
     Note: We need to access the module manager from the analyzer or pass it as a parameter
     For now, we'll generate IR for module functions that are in the semantic analyzer
     Only check for module functions if we have modules (when there are includes) */
    if (analyzer && analyzer->current_scope && ast_program->includes.size > 0)
    {
        if (debug_enabled)
        {
            printf("[DEBUG] ir_generate: Checking for module functions in semantic analyzer\n");
            printf("[DEBUG] ir_generate: Global scope has %zu buckets\n", analyzer->current_scope->symbols->capacity);
        }

        size_t bucket_count = 0;
        for (size_t i = 0; i < analyzer->current_scope->symbols->capacity; i++)
        {
            bucket_count++;
            if (debug_enabled && bucket_count % 5 == 0)
            {
                printf("[DEBUG] ir_generate: Processed %zu buckets\n", bucket_count);
            }

            HashTableEntry *entry = analyzer->current_scope->symbols->buckets[i];
            while (entry)
            {
                DynamicArray *overloads = (DynamicArray *)entry->value;
                if (overloads)
                {
                    for (size_t j = 0; j < overloads->size; j++)
                    {
                        Symbol *symbol = (Symbol *)array_get(overloads, j);
                        if (symbol->type == SYMBOL_FUNCTION)
                        {
                            if (symbol->data.function.params.size > 0)
                            {
                                if (debug_enabled)
                                {
                                    printf("[DEBUG] ir_generate: Found module function symbol: %s\n", symbol->name);
                                }

                                Function *module_func = function_create(symbol->name, symbol->data_type);

                                for (size_t k = 0; k < symbol->data.function.params.size; k++)
                                {
                                    Parameter *param = (Parameter *)array_get(&symbol->data.function.params, k);
                                    Parameter *param_copy = safe_malloc(sizeof(Parameter));
                                    param_copy->name = string_copy(param->name);
                                    param_copy->type = param->type;
                                    array_push(&module_func->params, param_copy);
                                }

                                // For now, create an empty body (module functions don't have bodies in symbols)
                                // The actual implementation will be added later
                                module_func->body = NULL;

                                if (debug_enabled)
                                {
                                    printf("[DEBUG] ir_generate: Generating IR for module function: %s\n", symbol->name);
                                }
                                IRFunction *ir_func = ir_generate_function(module_func, analyzer);
                                ir_program_add_function(ir_program, ir_func);

                                function_destroy(module_func);
                            }
                        }
                    }
                }
                entry = entry->next;
            }
        }

        if (debug_enabled)
        {
            printf("[DEBUG] ir_generate: Processed all %zu buckets\n", bucket_count);
        }
    }
    else if (debug_enabled)
    {
        printf("[DEBUG] ir_generate: Skipping module function check (no modules)\n");
    }

    if (debug_enabled)
    {
        printf("[DEBUG] ir_generate: Finished checking for module functions\n");
    }

    if (debug_enabled)
    {
        printf("[DEBUG] ir_generate: IR generation completed, program has %zu functions\n", ir_program->functions.size);
    }

    return ir_program;
}

IRFunction *ir_generate_function_impl(Function *func, SemanticAnalyzer *analyzer)
{
    IRFunction *ir_func = ir_function_create(func->name, func->return_type);

    for (size_t i = 0; i < func->params.size; i++)
    {
        Parameter *param = (Parameter *)array_get(&func->params, i);
        IROperand *param_op = ir_operand_var(param->name);
        param_op->data_type = param->type;
        ir_function_add_param(ir_func, param_op);
    }

    ir_generate_statement_impl(ir_func, func->body, analyzer);

    if (ir_func->oob_error_label)
    {
        IRInstruction *error_label_instr = ir_instruction_label(ir_func->oob_error_label);
        ir_function_add_instruction(ir_func, error_label_instr);
        
        IROperand *error_msg = ir_operand_string_const("Array index out of bounds");
        IRInstruction *runtime_error = ir_instruction_print_op(error_msg);
        ir_function_add_instruction(ir_func, runtime_error);
        
        IROperand *exit_code = ir_operand_const(1);
        IRInstruction *exit_instr = ir_instruction_return(exit_code);
        ir_function_add_instruction(ir_func, exit_instr);
    }

    return ir_func;
}
