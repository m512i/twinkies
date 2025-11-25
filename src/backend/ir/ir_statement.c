#include "backend/ir/ir.h"
#include "analysis/semantic/semantic.h"
#include "backend/ir/iroperands.h"
#include "backend/ir/irinstructions.h"
#include "common/common.h"
#include "modules/modules.h"

extern bool debug_enabled;

IROperand *ir_generate_expression_impl(IRFunction *ir_func, Expr *expr, SemanticAnalyzer *analyzer, DataType expected_type);
void ir_generate_statement_impl(IRFunction *ir_func, Stmt *stmt, SemanticAnalyzer *analyzer);


bool stmt_always_returns(Stmt *stmt)
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

void ir_generate_statement_impl(IRFunction *ir_func, Stmt *stmt, SemanticAnalyzer *analyzer)
{
    if (!stmt)
        return;

    switch (stmt->type)
    {
    case STMT_EXPR:
        ir_generate_expression_impl(ir_func, stmt->data.expr.expression, analyzer, TYPE_NULL);
        break;
    case STMT_VAR_DECL:
        IRInstruction *var_decl = ir_instruction_var_decl(stmt->data.var_decl.name, stmt->data.var_decl.type);
        ir_function_add_instruction(ir_func, var_decl);

        if (stmt->data.var_decl.initializer)
        {
            IROperand *value = ir_generate_expression_impl(ir_func, stmt->data.var_decl.initializer, analyzer, stmt->data.var_decl.type);
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
            IROperand *value = ir_generate_expression_impl(ir_func, stmt->data.array_decl.initializer, analyzer, TYPE_NULL);
            IRInstruction *array_init = ir_instruction_array_init(stmt->data.array_decl.name, stmt->data.array_decl.size, stmt->data.array_decl.element_type, value);
            ir_function_add_instruction(ir_func, array_init);
        }
        break;
    case STMT_ASSIGNMENT:
    {
        Symbol *symbol = scope_resolve(analyzer, stmt->data.assignment.name);
        DataType expected_type = symbol ? symbol->data_type : TYPE_NULL;
        IROperand *value = ir_generate_expression_impl(ir_func, stmt->data.assignment.value, analyzer, expected_type);
        IROperand *var = ir_operand_var(stmt->data.assignment.name);
        if (symbol)
        {
            var->data_type = symbol->data_type;
        }
        IRInstruction *move = ir_instruction_move(var, value);
        ir_function_add_instruction(ir_func, move);
        break;
    }
    case STMT_ARRAY_ASSIGNMENT:
    {
        IROperand *array = ir_generate_expression_impl(ir_func, stmt->data.array_assignment.array, analyzer, TYPE_NULL);
        IROperand *index = ir_generate_expression_impl(ir_func, stmt->data.array_assignment.index, analyzer, TYPE_NULL);
        IROperand *value = ir_generate_expression_impl(ir_func, stmt->data.array_assignment.value, analyzer, TYPE_NULL);

        int array_size = -1;
        if (array && array->type == IR_OP_VAR)
        {
            array_size = get_array_size(analyzer, array->data.var_name);
        }

        if (array_size != -1)
        {
            bool needs_bounds_check = true;
            
            if (index->type == IR_OP_CONST)
            {
                int index_value = index->data.const_value;
                if (index_value >= 0 && index_value < array_size)
                {
                    needs_bounds_check = false;
                }
            }
            
            if (needs_bounds_check)
            {
                if (!ir_func->oob_error_label)
                {
                    ir_func->oob_error_label = ir_function_new_label(ir_func);
                }
                
                IROperand *zero = ir_operand_const(0);
                IROperand *lower_condition = ir_operand_temp(ir_function_new_temp(ir_func));
                IRInstruction *lower_compare = ir_instruction_binary(IR_LT, lower_condition, index, zero);
                ir_function_add_instruction(ir_func, lower_compare);
                IRInstruction *lower_jump = ir_instruction_jump_if(lower_condition, ir_func->oob_error_label);
                ir_function_add_instruction(ir_func, lower_jump);
                
                IROperand *size = ir_operand_const(array_size);
                IROperand *upper_condition = ir_operand_temp(ir_function_new_temp(ir_func));
                IRInstruction *upper_compare = ir_instruction_binary(IR_GE, upper_condition, index, size);
                ir_function_add_instruction(ir_func, upper_compare);
                IRInstruction *upper_jump = ir_instruction_jump_if(upper_condition, ir_func->oob_error_label);
                ir_function_add_instruction(ir_func, upper_jump);
            }
        }

        IRInstruction *store = ir_instruction_array_store(array, index, value);
        ir_function_add_instruction(ir_func, store);
        break;
    }
    case STMT_IF:
    {
        char *then_label = ir_function_new_label(ir_func);
        char *end_label = ir_function_new_label(ir_func);

        IROperand *condition = ir_generate_expression_impl(ir_func, stmt->data.if_stmt.condition, analyzer, TYPE_BOOL);
        IRInstruction *jump_if_false = ir_instruction_jump_if_false(condition, then_label);
        ir_function_add_instruction(ir_func, jump_if_false);

        ir_generate_statement_impl(ir_func, stmt->data.if_stmt.then_branch, analyzer);
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

            ir_generate_statement_impl(ir_func, stmt->data.if_stmt.else_branch, analyzer);
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

        IROperand *condition = ir_generate_expression_impl(ir_func, stmt->data.while_stmt.condition, analyzer, TYPE_BOOL);
        IRInstruction *jump_if_false = ir_instruction_jump_if_false(condition, end_label);
        ir_function_add_instruction(ir_func, jump_if_false);

        ir_generate_statement_impl(ir_func, stmt->data.while_stmt.body, analyzer);

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
            IRInstruction *jump = ir_instruction_jump(current_loop->end_label);
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
            IRInstruction *jump = ir_instruction_jump(current_loop->start_label);
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
            IROperand *value = ir_generate_expression_impl(ir_func, stmt->data.return_stmt.value, analyzer, TYPE_NULL);
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
            IROperand *value = ir_generate_expression_impl(ir_func, arg, analyzer, TYPE_NULL);
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
                IROperand *value = ir_generate_expression_impl(ir_func, arg, analyzer, TYPE_NULL);
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
            ir_generate_statement_impl(ir_func, block_stmt, analyzer);
            if (stmt_always_returns(block_stmt))
                break;
        }
        break;
    case STMT_INCLUDE:
        // Include directives are handled during parsing, not IR generation
        // They don't generate any IR instructions
        break;
    case STMT_INLINE_ASM:
    {
        DynamicArray *outputs = safe_malloc(sizeof(DynamicArray));
        DynamicArray *inputs = safe_malloc(sizeof(DynamicArray));
        DynamicArray *clobbers = safe_malloc(sizeof(DynamicArray));
        array_init(outputs, sizeof(InlineAsmOperand*));
        array_init(inputs, sizeof(InlineAsmOperand*));
        array_init(clobbers, sizeof(char*));
        
        for (size_t i = 0; i < stmt->data.inline_asm.outputs.size; i++)
        {
            InlineAsmOperand *src = (InlineAsmOperand *)array_get(&stmt->data.inline_asm.outputs, i);
            InlineAsmOperand *dst = safe_malloc(sizeof(InlineAsmOperand));
            dst->constraint = string_copy(src->constraint);
            dst->variable = string_copy(src->variable);
            dst->is_output = true;
            array_push(outputs, dst);
        }
        
        for (size_t i = 0; i < stmt->data.inline_asm.inputs.size; i++)
        {
            InlineAsmOperand *src = (InlineAsmOperand *)array_get(&stmt->data.inline_asm.inputs, i);
            InlineAsmOperand *dst = safe_malloc(sizeof(InlineAsmOperand));
            dst->constraint = string_copy(src->constraint);
            dst->variable = string_copy(src->variable);
            dst->is_output = false;
            array_push(inputs, dst);
        }
        
        for (size_t i = 0; i < stmt->data.inline_asm.clobbers.size; i++)
        {
            char *src = (char *)array_get(&stmt->data.inline_asm.clobbers, i);
            char *dst = string_copy(src);
            array_push(clobbers, dst);
        }
        
        IRInstruction *asm_instr = ir_instruction_inline_asm(
            stmt->data.inline_asm.asm_code,
            stmt->data.inline_asm.is_volatile,
            outputs,
            inputs,
            clobbers
        );
        ir_function_add_instruction(ir_func, asm_instr);
        break;
    }
    }
}
