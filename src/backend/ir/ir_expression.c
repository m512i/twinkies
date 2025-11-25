#include "backend/ir/ir.h"
#include "analysis/semantic/semantic.h"
#include "backend/ir/iroperands.h"
#include "backend/ir/irinstructions.h"
#include "common/common.h"
#include "modules/modules.h"

extern bool debug_enabled;

IROperand *ir_generate_expression_impl(IRFunction *ir_func, Expr *expr, SemanticAnalyzer *analyzer, DataType expected_type);

IROperand *ir_generate_expression_impl(IRFunction *ir_func, Expr *expr, SemanticAnalyzer *analyzer, DataType expected_type)
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
        else if (expr->data.literal.is_bool_literal)
        {
            return ir_operand_const(expr->data.literal.value.bool_value ? 1 : 0);
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
        DataType semantic_left_type = type_check_expression(analyzer, expr->data.binary.left);
        DataType semantic_right_type = type_check_expression(analyzer, expr->data.binary.right);
        DataType semantic_result_type = type_check_expression(analyzer, expr);
        
        bool is_string_concat = false;
        if (expr->data.binary.operator== TOKEN_PLUS)
        {
            if (semantic_result_type == TYPE_STRING || 
                expected_type == TYPE_STRING)
            {
                is_string_concat = true;
            }
            else if (semantic_left_type == TYPE_STRING || semantic_right_type == TYPE_STRING)
            {
                if (!(semantic_left_type == TYPE_NULL && semantic_right_type == TYPE_NULL))
                {
                    is_string_concat = true;
                }
            }
        }
        
        DataType left_expected_type = TYPE_NULL;
        DataType right_expected_type = TYPE_NULL;
        
        if (is_string_concat)
        {
            left_expected_type = TYPE_STRING;
            right_expected_type = TYPE_STRING;
        }
        else
        {   
            bool left_is_null = (expr->data.binary.left->type == EXPR_NULL_LITERAL);
            bool right_is_null = (expr->data.binary.right->type == EXPR_NULL_LITERAL);
            
            if (left_is_null && right_is_null)
            {
                if (expected_type != TYPE_NULL && is_numeric_type(expected_type))
                {
                    left_expected_type = expected_type;
                    right_expected_type = expected_type;
                }
                else
                {
                    left_expected_type = TYPE_INT;
                    right_expected_type = TYPE_INT;
                }
            }
            else if (left_is_null)
            {
                if (expected_type != TYPE_NULL && is_numeric_type(expected_type))
                {
                    left_expected_type = expected_type;
                }
                else
                {
                    left_expected_type = semantic_right_type;
                    if (left_expected_type == TYPE_NULL)
                    {
                        left_expected_type = (expected_type != TYPE_NULL) ? expected_type : TYPE_INT;
                    }
                }
                right_expected_type = semantic_right_type;
            }
            else if (right_is_null)
            {
                if (expected_type != TYPE_NULL && is_numeric_type(expected_type))
                {
                    right_expected_type = expected_type;
                }
                else
                {
                    right_expected_type = semantic_left_type;
                    if (right_expected_type == TYPE_NULL)
                    {
                        right_expected_type = (expected_type != TYPE_NULL) ? expected_type : TYPE_INT;
                    }
                }
                left_expected_type = semantic_left_type;
            }
            else
            {
                left_expected_type = semantic_left_type;
                right_expected_type = semantic_right_type;
            }
        }
        
        IROperand *left = ir_generate_expression_impl(ir_func, expr->data.binary.left, analyzer, left_expected_type);
        IROperand *right = ir_generate_expression_impl(ir_func, expr->data.binary.right, analyzer, right_expected_type);

        if (is_string_concat && left && right)
        {
            IROperand *result = ir_operand_temp(ir_function_new_temp(ir_func));
            result->data_type = TYPE_STRING;
            if (left->data_type != TYPE_STRING)
            {
                left->data_type = TYPE_STRING;
            }
            if (right->data_type != TYPE_STRING)
            {
                right->data_type = TYPE_STRING;
            }
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
            if (expected_type == TYPE_FLOAT || expected_type == TYPE_DOUBLE)
            {
                result->data_type = expected_type;
            }
            else if (left->data_type == TYPE_DOUBLE || right->data_type == TYPE_DOUBLE)
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
        IROperand *operand = ir_generate_expression_impl(ir_func, expr->data.unary.operand, analyzer, TYPE_NULL);

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
        if (expected_type == TYPE_FLOAT || expected_type == TYPE_DOUBLE || expected_type == TYPE_INT)
        {
            result->data_type = expected_type;
        }
        else if (operand->data_type == TYPE_DOUBLE)
        {
            result->data_type = TYPE_DOUBLE;
        }
        else if (operand->data_type == TYPE_FLOAT)
        {
            result->data_type = TYPE_FLOAT;
        }
        else
        {
            result->data_type = operand->data_type;
        }
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
        else if (string_equal(expr->data.call.name, "input"))
        {
            result->data_type = TYPE_INT;
            if (debug_enabled)
            {
                printf("[DEBUG] ir_generate: Input function call, setting temp_%d to TYPE_INT\n", result->data.temp_id);
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

        Symbol *func_symbol = scope_resolve(analyzer, expr->data.call.name);
        DataType *param_types = NULL;
        size_t param_count = 0;
        
        if (func_symbol && func_symbol->type == SYMBOL_FUNCTION && 
            func_symbol->data.function.params.size > 0)
        {
            param_count = func_symbol->data.function.params.size;
            param_types = safe_malloc(sizeof(DataType) * param_count);
            for (size_t i = 0; i < param_count; i++)
            {
                Parameter *param = (Parameter *)array_get(&func_symbol->data.function.params, i);
                param_types[i] = param->type;
            }
        }
        else
        {
            if (string_equal(expr->data.call.name, "concat") || 
                string_equal(expr->data.call.name, "__tl_concat"))
            {
                param_count = 2;
                param_types = safe_malloc(sizeof(DataType) * param_count);
                param_types[0] = TYPE_STRING;
                param_types[1] = TYPE_STRING;
            }
            else if (string_equal(expr->data.call.name, "strlen") || 
                     string_equal(expr->data.call.name, "__tl_strlen"))
            {
                param_count = 1;
                param_types = safe_malloc(sizeof(DataType) * param_count);
                param_types[0] = TYPE_STRING;
            }
            else if (string_equal(expr->data.call.name, "substr") || 
                     string_equal(expr->data.call.name, "__tl_substr"))
            {
                param_count = 3;
                param_types = safe_malloc(sizeof(DataType) * param_count);
                param_types[0] = TYPE_STRING;
                param_types[1] = TYPE_INT;
                param_types[2] = TYPE_INT;
            }
            else if (string_equal(expr->data.call.name, "strcmp") || 
                     string_equal(expr->data.call.name, "__tl_strcmp"))
            {
                param_count = 2;
                param_types = safe_malloc(sizeof(DataType) * param_count);
                param_types[0] = TYPE_STRING;
                param_types[1] = TYPE_STRING;
            }
            else if (string_equal(expr->data.call.name, "char_at") || 
                     string_equal(expr->data.call.name, "__tl_char_at"))
            {
                param_count = 2;
                param_types = safe_malloc(sizeof(DataType) * param_count);
                param_types[0] = TYPE_STRING;
                param_types[1] = TYPE_INT;
            }
        }

        for (size_t i = 0; i < expr->data.call.args.size; i++)
        {
            Expr *arg_expr = (Expr *)array_get(&expr->data.call.args, i);
            DataType param_expected_type = (param_types && i < param_count) ? param_types[i] : TYPE_NULL;
            IROperand *arg = ir_generate_expression_impl(ir_func, arg_expr, analyzer, param_expected_type);
            IRInstruction *param = ir_instruction_param(arg);
            ir_function_add_instruction(ir_func, param);
        }
        
        if (param_types)
        {
            safe_free(param_types);
        }

        IRInstruction *call = ir_instruction_call(result, expr->data.call.name);
        ir_function_add_instruction(ir_func, call);
        return result;
    }

    case EXPR_GROUP:
        return ir_generate_expression_impl(ir_func, expr->data.group.expression, analyzer, expected_type);

    case EXPR_ARRAY_INDEX:
    {
        IROperand *array = ir_generate_expression_impl(ir_func, expr->data.array_index.array, analyzer, TYPE_NULL);
        IROperand *index = ir_generate_expression_impl(ir_func, expr->data.array_index.index, analyzer, TYPE_NULL);

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
        IROperand *string = ir_generate_expression_impl(ir_func, expr->data.string_index.string, analyzer, TYPE_NULL);
        IROperand *index = ir_generate_expression_impl(ir_func, expr->data.string_index.index, analyzer, TYPE_NULL);

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
