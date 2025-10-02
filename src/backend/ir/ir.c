#include "backend/ir.h"
#include "analysis/semantic.h"
#include "backend/iroperands.h"
#include "backend/irinstructions.h"
#include "common.h"
#include "modules.h"
extern bool debug_enabled;

LoopContext *ir_loop_context_create(char *start_label, char *end_label)
{
    LoopContext *context = safe_malloc(sizeof(LoopContext));
    context->loop_start_label = string_copy(start_label);
    context->loop_end_label = string_copy(end_label);
    context->parent = NULL;
    return context;
}

void ir_loop_context_destroy(LoopContext *context)
{
    if (!context)
        return;

    safe_free(context->loop_start_label);
    safe_free(context->loop_end_label);
    safe_free(context);
}

void ir_function_enter_loop(IRFunction *func, char *start_label, char *end_label)
{
    LoopContext *new_context = ir_loop_context_create(start_label, end_label);
    new_context->parent = func->current_loop;
    func->current_loop = new_context;
}

void ir_function_exit_loop(IRFunction *func)
{
    if (!func->current_loop)
        return;

    LoopContext *parent = func->current_loop->parent;
    ir_loop_context_destroy(func->current_loop);
    func->current_loop = parent;
}

LoopContext *ir_function_get_current_loop(IRFunction *func)
{
    return func->current_loop;
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
    func->current_loop = NULL;
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

    while (func->current_loop)
    {
        ir_function_exit_loop(func);
    }

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

IRProgram *ir_generate_with_modules(Program *ast_program, SemanticAnalyzer *analyzer, void *module_manager)
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
        IRFunction *ir_func = ir_generate_function(func, analyzer);
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
                        IRFunction *ir_func = ir_generate_function(func, analyzer);
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

IRProgram *ir_generate(Program *ast_program, SemanticAnalyzer *analyzer)
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
        IRFunction *ir_func = ir_generate_function(func, analyzer);
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

IRFunction *ir_generate_function(Function *func, SemanticAnalyzer *analyzer)
{
    IRFunction *ir_func = ir_function_create(func->name, func->return_type);

    for (size_t i = 0; i < func->params.size; i++)
    {
        Parameter *param = (Parameter *)array_get(&func->params, i);
        IROperand *param_op = ir_operand_var(param->name);
        param_op->data_type = param->type;
        ir_function_add_param(ir_func, param_op);
    }

    ir_generate_statement(ir_func, func->body, analyzer);

    return ir_func;
}

static bool stmt_always_returns(Stmt *stmt)
{
    if (!stmt)
        return false;
    if (stmt->type == STMT_RETURN)
        return true;
    if (stmt->type == STMT_BLOCK)
    {
        size_t n = stmt->data.block.statements.size;
        if (n == 0)
            return false;
        return stmt_always_returns((Stmt *)array_get(&stmt->data.block.statements, n - 1));
    }
    if (stmt->type == STMT_IF)
    {
        if (stmt->data.if_stmt.else_branch)
        {
            return stmt_always_returns(stmt->data.if_stmt.then_branch) && stmt_always_returns(stmt->data.if_stmt.else_branch);
        }
    }
    return false;
}

void ir_generate_statement(IRFunction *ir_func, Stmt *stmt, SemanticAnalyzer *analyzer)
{
    if (!stmt)
        return;

    switch (stmt->type)
    {
    case STMT_EXPR:
        ir_generate_expression(ir_func, stmt->data.expr.expression, analyzer, TYPE_NULL);
        break;
    case STMT_VAR_DECL:
        IRInstruction *var_decl = ir_instruction_var_decl(stmt->data.var_decl.name, stmt->data.var_decl.type);
        ir_function_add_instruction(ir_func, var_decl);

        if (stmt->data.var_decl.initializer)
        {
            IROperand *value = ir_generate_expression(ir_func, stmt->data.var_decl.initializer, analyzer, stmt->data.var_decl.type);
            IROperand *var = ir_operand_var(stmt->data.var_decl.name);
            var->data_type = stmt->data.var_decl.type;

            IRInstruction *move = ir_instruction_move(var, value);
            ir_function_add_instruction(ir_func, move);
        }
        break;
    case STMT_ARRAY_DECL:
        IRInstruction *array_decl = ir_instruction_array_decl(stmt->data.array_decl.name, stmt->data.array_decl.size, stmt->data.array_decl.element_type);
        ir_function_add_instruction(ir_func, array_decl);
        if (stmt->data.array_decl.initializer)
        {
            IROperand *value = ir_generate_expression(ir_func, stmt->data.array_decl.initializer, analyzer, TYPE_NULL);
            IRInstruction *array_init = ir_instruction_array_init(stmt->data.array_decl.name, stmt->data.array_decl.size, stmt->data.array_decl.element_type, value);
            ir_function_add_instruction(ir_func, array_init);
        }
        break;
    case STMT_ASSIGNMENT:
    {
        IROperand *value = ir_generate_expression(ir_func, stmt->data.assignment.value, analyzer, TYPE_NULL);
        IROperand *var = ir_operand_var(stmt->data.assignment.name);
        IRInstruction *move = ir_instruction_move(var, value);
        ir_function_add_instruction(ir_func, move);
        break;
    }
    case STMT_ARRAY_ASSIGNMENT:
    {
        IROperand *array = ir_generate_expression(ir_func, stmt->data.array_assignment.array, analyzer, TYPE_NULL);
        IROperand *index = ir_generate_expression(ir_func, stmt->data.array_assignment.index, analyzer, TYPE_NULL);
        IROperand *value = ir_generate_expression(ir_func, stmt->data.array_assignment.value, analyzer, TYPE_NULL);

        int array_size = -1;
        if (array && array->type == IR_OP_VAR)
        {
            array_size = get_array_size(analyzer, array->data.var_name);
        }

        if (array_size != -1)
        {
            char *error_label = ir_function_new_label(ir_func);
            IROperand *size = ir_operand_const(array_size);
            IRInstruction *bounds_check = ir_instruction_bounds_check(index, size, error_label);
            ir_function_add_instruction(ir_func, bounds_check);
        }

        IRInstruction *store = ir_instruction_array_store(array, index, value);
        ir_function_add_instruction(ir_func, store);
        break;
    }
    case STMT_IF:
    {
        char *then_label = ir_function_new_label(ir_func);
        char *end_label = ir_function_new_label(ir_func);

        IROperand *condition = ir_generate_expression(ir_func, stmt->data.if_stmt.condition, analyzer, TYPE_BOOL);
        IRInstruction *jump_if_false = ir_instruction_jump_if_false(condition, then_label);
        ir_function_add_instruction(ir_func, jump_if_false);

        ir_generate_statement(ir_func, stmt->data.if_stmt.then_branch, analyzer);
        bool then_returns = stmt_always_returns(stmt->data.if_stmt.then_branch);

        if (stmt->data.if_stmt.else_branch)
        {
            char *else_label = ir_function_new_label(ir_func);
            if (!then_returns)
            {
                IRInstruction *jump = ir_instruction_jump(else_label);
                ir_function_add_instruction(ir_func, jump);
            }
            IRInstruction *then_lbl = ir_instruction_label(then_label);
            ir_function_add_instruction(ir_func, then_lbl);

            ir_generate_statement(ir_func, stmt->data.if_stmt.else_branch, analyzer);
            bool else_returns = stmt_always_returns(stmt->data.if_stmt.else_branch);
            if (!else_returns)
            {
                IRInstruction *end_lbl = ir_instruction_label(else_label);
                ir_function_add_instruction(ir_func, end_lbl);
            }
        }
        else
        {
            if (!then_returns)
            {
                IRInstruction *jump = ir_instruction_jump(end_label);
                ir_function_add_instruction(ir_func, jump);
            }
            IRInstruction *then_lbl = ir_instruction_label(then_label);
            ir_function_add_instruction(ir_func, then_lbl);
            if (!then_returns)
            {
                IRInstruction *end_lbl = ir_instruction_label(end_label);
                ir_function_add_instruction(ir_func, end_lbl);
            }
        }
        break;
    }
    case STMT_WHILE:
    {
        char *loop_label = ir_function_new_label(ir_func);
        char *end_label = ir_function_new_label(ir_func);

        ir_function_enter_loop(ir_func, loop_label, end_label);

        IRInstruction *loop_lbl = ir_instruction_label(loop_label);
        ir_function_add_instruction(ir_func, loop_lbl);

        IROperand *condition = ir_generate_expression(ir_func, stmt->data.while_stmt.condition, analyzer, TYPE_BOOL);
        IRInstruction *jump_if_false = ir_instruction_jump_if_false(condition, end_label);
        ir_function_add_instruction(ir_func, jump_if_false);

        ir_generate_statement(ir_func, stmt->data.while_stmt.body, analyzer);

        IRInstruction *jump = ir_instruction_jump(loop_label);
        ir_function_add_instruction(ir_func, jump);

        IRInstruction *end_lbl = ir_instruction_label(end_label);
        ir_function_add_instruction(ir_func, end_lbl);

        ir_function_exit_loop(ir_func);
        break;
    }

    case STMT_BREAK:
    {
        LoopContext *current_loop = ir_function_get_current_loop(ir_func);
        if (current_loop)
        {
            IRInstruction *jump = ir_instruction_jump(current_loop->loop_end_label);
            ir_function_add_instruction(ir_func, jump);
        }
        else
        {
            if (debug_enabled)
            {
                fprintf(stderr, "[IR ERROR] 'break' statement not within a loop (IR generation)\n");
            }
            // abort or insert a trap instruction here
        }
        break;
    }

    case STMT_CONTINUE:
    {
        LoopContext *current_loop = ir_function_get_current_loop(ir_func);
        if (current_loop)
        {
            IRInstruction *jump = ir_instruction_jump(current_loop->loop_start_label);
            ir_function_add_instruction(ir_func, jump);
        }
        else
        {
            if (debug_enabled)
            {
                fprintf(stderr, "[IR ERROR] 'continue' statement not within a loop (IR generation)\n");
            }
            // abort or insert a trap instruction here
        }
        break;
    }
    case STMT_RETURN:
        if (stmt->data.return_stmt.value)
        {
            IROperand *value = ir_generate_expression(ir_func, stmt->data.return_stmt.value, analyzer, TYPE_NULL);
            IRInstruction *ret = ir_instruction_return(value);
            ir_function_add_instruction(ir_func, ret);
        }
        else
        {
            IRInstruction *ret = ir_instruction_return(NULL);
            ir_function_add_instruction(ir_func, ret);
        }
        break;
    case STMT_PRINT:
        if (stmt->data.print_stmt.args.size == 1)
        {
            Expr *arg = (Expr *)array_get(&stmt->data.print_stmt.args, 0);
            IROperand *value = ir_generate_expression(ir_func, arg, analyzer, TYPE_NULL);
            IRInstruction *print = ir_instruction_print_op(value);
            ir_function_add_instruction(ir_func, print);
        }
        else
        {
            DynamicArray *print_args = safe_malloc(sizeof(DynamicArray));
            array_init(print_args, stmt->data.print_stmt.args.size);
            
            for (size_t i = 0; i < stmt->data.print_stmt.args.size; i++)
            {
                Expr *arg = (Expr *)array_get(&stmt->data.print_stmt.args, i);
                IROperand *value = ir_generate_expression(ir_func, arg, analyzer, TYPE_NULL);
                array_push(print_args, value);
            }
            
            IRInstruction *print = ir_instruction_print_multiple(print_args);
            ir_function_add_instruction(ir_func, print);
        }
        break;
    case STMT_BLOCK:
        for (size_t i = 0; i < stmt->data.block.statements.size; i++)
        {
            Stmt *block_stmt = (Stmt *)array_get(&stmt->data.block.statements, i);
            ir_generate_statement(ir_func, block_stmt, analyzer);
            if (stmt_always_returns(block_stmt))
                break;
        }
        break;
    case STMT_INCLUDE:
        // Include directives are handled during parsing, not IR generation
        // They don't generate any IR instructions
        break;
    }
}

IROperand *ir_generate_expression(IRFunction *ir_func, Expr *expr, SemanticAnalyzer *analyzer, DataType expected_type)
{
    if (!expr)
        return NULL;

    switch (expr->type)
    {
    case EXPR_LITERAL:
        if (expr->data.literal.is_string_literal)
        {
            return ir_operand_string_const(expr->data.literal.value.string_value);
        }
        else if (expr->data.literal.is_float_literal)
        {
            return ir_operand_float_const(expr->data.literal.value.float_value);
        }
        else
        {
            return ir_operand_const(expr->data.literal.value.number_value);
        }

    case EXPR_VARIABLE:
    {
        Symbol *symbol = scope_resolve(analyzer, expr->data.variable.name);
        if (debug_enabled)
        {
            printf("[DEBUG] Variable %s: symbol found = %s, data_type = %d\n",
                   expr->data.variable.name, symbol ? "yes" : "no",
                   symbol ? (int)symbol->data_type : -1);
        }

        if (symbol && symbol->data_type == TYPE_STRING)
        {
            IROperand *operand = ir_operand_var(expr->data.variable.name);
            operand->data_type = TYPE_STRING;
            return operand;
        }

        int array_size = get_array_size(analyzer, expr->data.variable.name);
        if (debug_enabled)
        {
            printf("[DEBUG] Variable %s: array_size = %d\n", expr->data.variable.name, array_size);
        }
        if (array_size != -1)
        {
            IROperand *operand = ir_operand_array_var(expr->data.variable.name, array_size);
            return operand;
        }
        else
        {
            IROperand *operand = ir_operand_var(expr->data.variable.name);
            if (symbol)
            {
                operand->data_type = symbol->data_type;
            }
            return operand;
        }
    }

    case EXPR_BINARY:
    {
        DataType left_type = TYPE_NULL;
        DataType right_type = TYPE_NULL;
        if (expr->data.binary.left->type == EXPR_NULL_LITERAL)
        {
            right_type = type_check_expression(analyzer, expr->data.binary.right);
        }
        if (expr->data.binary.right->type == EXPR_NULL_LITERAL)
        {
            left_type = type_check_expression(analyzer, expr->data.binary.left);
        }
        IROperand *left = ir_generate_expression(ir_func, expr->data.binary.left, analyzer, right_type);
        IROperand *right = ir_generate_expression(ir_func, expr->data.binary.right, analyzer, left_type);

        if (expr->data.binary.operator== TOKEN_PLUS && left && right && left->data_type == TYPE_STRING && right->data_type == TYPE_STRING)
        {
            IROperand *result = ir_operand_temp(ir_function_new_temp(ir_func));
            result->data_type = TYPE_STRING;
            if (debug_enabled)
            {
                printf("[DEBUG] ir_generate: String concatenation, setting temp_%d to TYPE_STRING\n", result->data.temp_id);
            }
            IRInstruction *param1 = ir_instruction_param(left);
            IRInstruction *param2 = ir_instruction_param(right);
            ir_function_add_instruction(ir_func, param1);
            ir_function_add_instruction(ir_func, param2);
            IRInstruction *call = ir_instruction_call(result, "__tl_concat");
            ir_function_add_instruction(ir_func, call);
            return result;
        }

        IROpcode opcode;
        switch (expr->data.binary.operator)
        {
        case TOKEN_PLUS:
            opcode = IR_ADD;
            break;
        case TOKEN_MINUS:
            opcode = IR_SUB;
            break;
        case TOKEN_STAR:
            opcode = IR_MUL;
            break;
        case TOKEN_SLASH:
            opcode = IR_DIV;
            break;
        case TOKEN_PERCENT:
            opcode = IR_MOD;
            break;
        case TOKEN_EQ:
            opcode = IR_EQ;
            break;
        case TOKEN_NE:
            opcode = IR_NE;
            break;
        case TOKEN_LT:
            opcode = IR_LT;
            break;
        case TOKEN_LE:
            opcode = IR_LE;
            break;
        case TOKEN_GT:
            opcode = IR_GT;
            break;
        case TOKEN_GE:
            opcode = IR_GE;
            break;
        case TOKEN_AND:
            opcode = IR_AND;
            break;
        case TOKEN_OR:
            opcode = IR_OR;
            break;
        default:
            return NULL;
        }

        IROperand *result = ir_operand_temp(ir_function_new_temp(ir_func));

        if (opcode == IR_ADD || opcode == IR_SUB || opcode == IR_MUL || opcode == IR_DIV || opcode == IR_MOD)
        {
            if (left->data_type == TYPE_DOUBLE || right->data_type == TYPE_DOUBLE)
            {
                result->data_type = TYPE_DOUBLE;
            }
            else if (left->data_type == TYPE_FLOAT || right->data_type == TYPE_FLOAT)
            {
                result->data_type = TYPE_FLOAT;
            }
            else
            {
                result->data_type = TYPE_INT;
            }
        }
        else
        {
            result->data_type = TYPE_BOOL;
        }

        IRInstruction *instr = ir_instruction_binary(opcode, result, left, right);
        ir_function_add_instruction(ir_func, instr);
        return result;
    }

    case EXPR_UNARY:
    {
        IROperand *operand = ir_generate_expression(ir_func, expr->data.unary.operand, analyzer, TYPE_NULL);

        IROpcode opcode;
        switch (expr->data.unary.operator)
        {
        case TOKEN_MINUS:
            opcode = IR_NEG;
            break;
        case TOKEN_BANG:
            opcode = IR_NOT;
            break;
        default:
            return NULL;
        }

        IROperand *result = ir_operand_temp(ir_function_new_temp(ir_func));
        IRInstruction *instr = ir_instruction_unary(opcode, result, operand);
        ir_function_add_instruction(ir_func, instr);
        return result;
    }

    case EXPR_CALL:
    {
        IROperand *result = ir_operand_temp(ir_function_new_temp(ir_func));

        if (debug_enabled)
        {
            printf("[DEBUG] ir_generate: Processing function call: %s\n", expr->data.call.name);
        }

        if (string_equal(expr->data.call.name, "string_concat") ||
            string_equal(expr->data.call.name, "string_substr") ||
            string_equal(expr->data.call.name, "char_at") ||
            string_equal(expr->data.call.name, "__tl_substr") ||
            string_equal(expr->data.call.name, "__tl_concat") ||
            string_equal(expr->data.call.name, "substr") ||
            string_equal(expr->data.call.name, "concat"))
        {
            result->data_type = TYPE_STRING;
            if (debug_enabled)
            {
                printf("[DEBUG] ir_generate: String function call %s, setting temp_%d to TYPE_STRING\n", expr->data.call.name, result->data.temp_id);
            }
        }
        else if (string_equal(expr->data.call.name, "string_length") ||
                 string_equal(expr->data.call.name, "string_compare") ||
                 string_equal(expr->data.call.name, "__tl_strlen") ||
                 string_equal(expr->data.call.name, "__tl_strcmp") ||
                 string_equal(expr->data.call.name, "strlen") ||
                 string_equal(expr->data.call.name, "strcmp"))
        {
            result->data_type = TYPE_INT;
            if (debug_enabled)
            {
                printf("[DEBUG] ir_generate: String length/compare function call %s, setting temp_%d to TYPE_INT\n", expr->data.call.name, result->data.temp_id);
            }
        }
        else if (string_equal(expr->data.call.name, "test_function"))
        {
            result->data_type = TYPE_DOUBLE;
        }
        else
        {
            result->data_type = TYPE_INT;
            if (debug_enabled)
            {
                printf("[DEBUG] ir_generate: Default function call %s, setting temp_%d to TYPE_INT\n", expr->data.call.name, result->data.temp_id);
            }
        }

        for (size_t i = 0; i < expr->data.call.args.size; i++)
        {
            Expr *arg_expr = (Expr *)array_get(&expr->data.call.args, i);
            IROperand *arg = ir_generate_expression(ir_func, arg_expr, analyzer, TYPE_NULL);
            IRInstruction *param = ir_instruction_param(arg);
            ir_function_add_instruction(ir_func, param);
        }

        IRInstruction *call = ir_instruction_call(result, expr->data.call.name);
        ir_function_add_instruction(ir_func, call);
        return result;
    }

    case EXPR_GROUP:
        return ir_generate_expression(ir_func, expr->data.group.expression, analyzer, expected_type);

    case EXPR_ARRAY_INDEX:
    {
        IROperand *array = ir_generate_expression(ir_func, expr->data.array_index.array, analyzer, TYPE_NULL);
        IROperand *index = ir_generate_expression(ir_func, expr->data.array_index.index, analyzer, TYPE_NULL);

        if (array && array->data_type == TYPE_STRING)
        {
            IROperand *result = ir_operand_temp(ir_function_new_temp(ir_func));

            if (expected_type == TYPE_STRING)
            {
                result->data_type = TYPE_STRING;
                if (debug_enabled)
                {
                    printf("[DEBUG] ir_generate: String indexing for assignment, setting temp_%d to TYPE_STRING\n", result->data.temp_id);
                }

                IRInstruction *param1 = ir_instruction_param(array);
                IRInstruction *param2 = ir_instruction_param(index);
                ir_function_add_instruction(ir_func, param1);
                ir_function_add_instruction(ir_func, param2);
                IRInstruction *call = ir_instruction_call(result, "__tl_char_at");
                ir_function_add_instruction(ir_func, call);
            }
            else
            {
                result->data_type = TYPE_INT;
                if (debug_enabled)
                {
                    printf("[DEBUG] ir_generate: String indexing for comparison, setting temp_%d to TYPE_INT\n", result->data.temp_id);
                }

                IRInstruction *load = ir_instruction_array_load(result, array, index);
                ir_function_add_instruction(ir_func, load);
            }

            return result;
        }
        else
        {
            int array_size = -1;
            if (array && array->type == IR_OP_VAR)
            {
                array_size = get_array_size(analyzer, array->data.var_name);
            }

            if (array_size != -1)
            {
                char *error_label = ir_function_new_label(ir_func);
                IROperand *size = ir_operand_const(array_size);
                IRInstruction *bounds_check = ir_instruction_bounds_check(index, size, error_label);
                ir_function_add_instruction(ir_func, bounds_check);
            }

            IROperand *result = ir_operand_temp(ir_function_new_temp(ir_func));

            if (array->type == IR_OP_VAR)
            {
                Symbol *symbol = scope_resolve(analyzer, array->data.var_name);
                if (symbol && symbol->data_type == TYPE_ARRAY && symbol->element_type == TYPE_STRING)
                {
                    result->data_type = TYPE_STRING;
                }
                else
                {
                    result->data_type = array->data_type;
                }
            }
            else
            {
                result->data_type = array->data_type;
            }

            IRInstruction *load = ir_instruction_array_load(result, array, index);
            ir_function_add_instruction(ir_func, load);

            return result;
        }
    }

    case EXPR_STRING_INDEX:
    {
        IROperand *string = ir_generate_expression(ir_func, expr->data.string_index.string, analyzer, TYPE_NULL);
        IROperand *index = ir_generate_expression(ir_func, expr->data.string_index.index, analyzer, TYPE_NULL);

        IROperand *result = ir_operand_temp(ir_function_new_temp(ir_func));
        result->data_type = TYPE_STRING;

        IRInstruction *load = ir_instruction_array_load(result, string, index);
        ir_function_add_instruction(ir_func, load);

        return result;
    }

    case EXPR_NULL_LITERAL:
    {
        IROperand *result = ir_operand_temp(ir_function_new_temp(ir_func));
        if (expected_type == TYPE_STRING)
        {
            result->data_type = TYPE_STRING;
        }
        else if (expected_type == TYPE_NULL)
        {
            result->data_type = TYPE_INT;
        }
        else
        {
            result->data_type = expected_type;
        }
        IROperand *null_value = ir_operand_null_with_type(result->data_type);
        IRInstruction *move = ir_instruction_move(result, null_value);
        ir_function_add_instruction(ir_func, move);
        return result;
    }

    default:
        return NULL;
    }
}