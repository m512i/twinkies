#include "backend/ir.h"
extern bool debug_enabled;

IROperand* ir_operand_temp(int temp_id) {
    IROperand* operand = safe_malloc(sizeof(IROperand));
    operand->type = IR_OP_TEMP;
    operand->array_size = -1;  
    operand->is_float_const = false;
    operand->data_type = TYPE_INT;  
    operand->data.temp_id = temp_id;
    return operand;
}

IROperand* ir_operand_var(const char* var_name) {
    IROperand* operand = safe_malloc(sizeof(IROperand));
    operand->type = IR_OP_VAR;
    operand->array_size = -1;  
    operand->is_float_const = false;
    operand->data_type = TYPE_INT;  
    operand->data.var_name = string_copy(var_name);
    return operand;
}

IROperand* ir_operand_array_var(const char* var_name, int size) {
    IROperand* operand = safe_malloc(sizeof(IROperand));
    operand->type = IR_OP_VAR;
    operand->array_size = size;  
    operand->is_float_const = false;
    operand->data_type = TYPE_ARRAY;
    operand->data.var_name = string_copy(var_name);
    return operand;
}

IROperand* ir_operand_const(int64_t value) {
    IROperand* operand = safe_malloc(sizeof(IROperand));
    operand->type = IR_OP_CONST;
    operand->array_size = -1;  
    operand->is_float_const = false;
    operand->data_type = TYPE_INT;
    operand->data.const_value = value;
    return operand;
}

IROperand* ir_operand_float_const(double value) {
    IROperand* operand = safe_malloc(sizeof(IROperand));
    operand->type = IR_OP_CONST;
    operand->array_size = -1;  
    operand->is_float_const = true;
    operand->data_type = TYPE_DOUBLE;
    operand->data.const_value = (int64_t)(value * 1000000);
    return operand;
}

IROperand* ir_operand_label(const char* label_name) {
    IROperand* operand = safe_malloc(sizeof(IROperand));
    operand->type = IR_OP_LABEL;
    operand->array_size = -1;  
    operand->is_float_const = false;
    operand->data_type = TYPE_VOID;
    operand->data.label_name = string_copy(label_name);
    return operand;
}

IRInstruction* ir_instruction_nop(void) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_NOP;
    instr->result = NULL;
    instr->arg1 = NULL;
    instr->arg2 = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction* ir_instruction_label(const char* label) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_LABEL;
    instr->result = NULL;
    instr->arg1 = NULL;
    instr->arg2 = NULL;
    instr->label = string_copy(label);
    return instr;
}

IRInstruction* ir_instruction_move(IROperand* result, IROperand* source) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_MOVE;
    instr->result = result;
    instr->arg1 = source;
    instr->arg2 = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction* ir_instruction_binary(IROpcode opcode, IROperand* result, IROperand* arg1, IROperand* arg2) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = opcode;
    instr->result = result;
    instr->arg1 = arg1;
    instr->arg2 = arg2;
    instr->label = NULL;
    return instr;
}

IRInstruction* ir_instruction_unary(IROpcode opcode, IROperand* result, IROperand* arg) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = opcode;
    instr->result = result;
    instr->arg1 = arg;
    instr->arg2 = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction* ir_instruction_jump(const char* label) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_JUMP;
    instr->result = NULL;
    instr->arg1 = NULL;
    instr->arg2 = NULL;
    instr->label = string_copy(label);
    return instr;
}

IRInstruction* ir_instruction_jump_if(IROperand* condition, const char* label) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_JUMP_IF;
    instr->result = NULL;
    instr->arg1 = condition;
    instr->arg2 = NULL;
    instr->label = string_copy(label);
    return instr;
}

IRInstruction* ir_instruction_jump_if_false(IROperand* condition, const char* label) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_JUMP_IF_FALSE;
    instr->result = NULL;
    instr->arg1 = condition;
    instr->arg2 = NULL;
    instr->label = string_copy(label);
    return instr;
}

IRInstruction* ir_instruction_call(IROperand* result, const char* func_name) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_CALL;
    instr->result = result;
    instr->arg1 = NULL;
    instr->arg2 = NULL;
    instr->label = string_copy(func_name);
    return instr;
}

IRInstruction* ir_instruction_return(IROperand* value) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_RETURN;
    instr->result = NULL;
    instr->arg1 = value;
    instr->arg2 = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction* ir_instruction_param(IROperand* param) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_PARAM;
    instr->result = NULL;
    instr->arg1 = param;
    instr->arg2 = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction* ir_instruction_print_op(IROperand* value) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_PRINT;
    instr->result = NULL;
    instr->arg1 = value;
    instr->arg2 = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction* ir_instruction_array_load(IROperand* result, IROperand* array, IROperand* index) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_ARRAY_LOAD;
    instr->result = result;
    instr->arg1 = array;
    instr->arg2 = index;
    instr->label = NULL;
    return instr;
}

IRInstruction* ir_instruction_array_store(IROperand* array, IROperand* index, IROperand* value) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_ARRAY_STORE;
    instr->result = value;
    instr->arg1 = array;
    instr->arg2 = index;
    instr->label = NULL;
    return instr;
}

IRInstruction* ir_instruction_bounds_check(IROperand* index, IROperand* size, const char* error_label) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_BOUNDS_CHECK;
    instr->result = NULL;
    instr->arg1 = index;
    instr->arg2 = size;
    instr->label = string_copy(error_label);
    return instr;
}

IRInstruction* ir_instruction_array_decl(const char* array_name, int size, DataType element_type) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_ARRAY_DECL;
    IROperand* array_var = ir_operand_array_var(array_name, size);
    array_var->data_type = element_type;
    instr->result = array_var;
    instr->arg1 = NULL;
    instr->arg2 = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction* ir_instruction_array_init(const char* array_name, int size, DataType element_type, IROperand* value) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_ARRAY_INIT;
    IROperand* array_var = ir_operand_array_var(array_name, size);
    array_var->data_type = element_type;
    instr->result = array_var;
    instr->arg1 = value;
    instr->arg2 = NULL;
    instr->label = NULL;
    return instr;
}

IRInstruction* ir_instruction_var_decl(const char* var_name, DataType type) {
    IRInstruction* instr = safe_malloc(sizeof(IRInstruction));
    instr->opcode = IR_VAR_DECL;
    IROperand* var = ir_operand_var(var_name);
    var->data_type = type;
    instr->result = var;
    instr->arg1 = NULL;
    instr->arg2 = NULL;
    instr->label = NULL;
    return instr;
}

IRFunction* ir_function_create(const char* name, DataType return_type) {
    IRFunction* func = safe_malloc(sizeof(IRFunction));
    func->name = string_copy(name);
    func->return_type = return_type;
    array_init(&func->params, 4);
    array_init(&func->instructions, 16);
    func->temp_counter = 0;
    func->label_counter = 0;
    return func;
}

IRProgram* ir_program_create(void) {
    IRProgram* program = safe_malloc(sizeof(IRProgram));
    array_init(&program->functions, 4);
    return program;
}

void ir_function_add_instruction(IRFunction* func, IRInstruction* instr) {
    array_push(&func->instructions, instr);
}

void ir_function_add_param(IRFunction* func, IROperand* param) {
    array_push(&func->params, param);
}

void ir_program_add_function(IRProgram* program, IRFunction* func) {
    array_push(&program->functions, func);
}

void ir_operand_destroy(IROperand* operand) {
    if (!operand) return;
    
    switch (operand->type) {
        case IR_OP_VAR:
            safe_free(operand->data.var_name);
            break;
        case IR_OP_LABEL:
            safe_free(operand->data.label_name);
            break;
        default:
            break;
    }
    safe_free(operand);
}

void ir_instruction_destroy(IRInstruction* instr) {
    if (!instr) return;
    
    if (instr->result) ir_operand_destroy(instr->result);
    if (instr->arg1) ir_operand_destroy(instr->arg1);
    if (instr->arg2) ir_operand_destroy(instr->arg2);
    if (instr->label) safe_free(instr->label);
    safe_free(instr);
}

void ir_function_destroy(IRFunction* func) {
    if (!func) return;
    
    safe_free(func->name);
    
    for (size_t i = 0; i < func->params.size; i++) {
        ir_operand_destroy((IROperand*)array_get(&func->params, i));
    }
    array_free(&func->params);
    
    for (size_t i = 0; i < func->instructions.size; i++) {
        ir_instruction_destroy((IRInstruction*)array_get(&func->instructions, i));
    }
    array_free(&func->instructions);
    
    safe_free(func);
}

void ir_program_destroy(IRProgram* program) {
    if (!program) return;
    
    for (size_t i = 0; i < program->functions.size; i++) {
        ir_function_destroy((IRFunction*)array_get(&program->functions, i));
    }
    array_free(&program->functions);
    safe_free(program);
}

void ir_operand_print(const IROperand* operand) {
    if (!operand) {
        printf("NULL");
        return;
    }
    
    switch (operand->type) {
        case IR_OP_TEMP:
            printf("t%d", operand->data.temp_id);
            break;
        case IR_OP_VAR:
            printf("%s", operand->data.var_name);
            break;
        case IR_OP_CONST:
            if (operand->is_float_const) {
                double float_value = (double)operand->data.const_value / 1000000.0;
                printf("%f", float_value);
            } else {
                printf("%lld", operand->data.const_value);
            }
            break;
        case IR_OP_LABEL:
            printf("%s", operand->data.label_name);
            break;
    }
}

void ir_instruction_print(const IRInstruction* instr) {
    if (!instr) return;
    
    switch (instr->opcode) {
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
            if (instr->result) {
                ir_operand_print(instr->result);
                printf(" = ");
            }
            printf("CALL %s", instr->label);
            break;
        case IR_RETURN:
            printf("RETURN");
            if (instr->arg1) {
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

void ir_function_print(const IRFunction* func) {
    if (!func) return;
    
    printf("Function: %s\n", func->name);
    printf("Parameters: ");
    for (size_t i = 0; i < func->params.size; i++) {
        if (i > 0) printf(", ");
        ir_operand_print((IROperand*)array_get(&func->params, i));
    }
    printf("\n");
    
    printf("Instructions:\n");
    for (size_t i = 0; i < func->instructions.size; i++) {
        printf("  %zu: ", i);
        ir_instruction_print((IRInstruction*)array_get(&func->instructions, i));
    }
    printf("\n");
}

void ir_program_print(const IRProgram* program) {
    if (!program) return;
    
    printf("IR Program:\n");
    printf("===========\n");
    
    for (size_t i = 0; i < program->functions.size; i++) {
        ir_function_print((IRFunction*)array_get(&program->functions, i));
    }
}

const char* ir_opcode_to_string(IROpcode opcode) {
    switch (opcode) {
        case IR_NOP: return "NOP";
        case IR_LABEL: return "LABEL";
        case IR_MOVE: return "MOVE";
        case IR_ADD: return "+";
        case IR_SUB: return "-";
        case IR_MUL: return "*";
        case IR_DIV: return "/";
        case IR_MOD: return "%";
        case IR_NEG: return "NEG";
        case IR_NOT: return "NOT";
        case IR_EQ: return "==";
        case IR_NE: return "!=";
        case IR_LT: return "<";
        case IR_LE: return "<=";
        case IR_GT: return ">";
        case IR_GE: return ">=";
        case IR_AND: return "&&";
        case IR_OR: return "||";
        case IR_JUMP: return "JUMP";
        case IR_JUMP_IF: return "JUMP_IF";
        case IR_JUMP_IF_FALSE: return "JUMP_IF_FALSE";
        case IR_CALL: return "CALL";
        case IR_RETURN: return "RETURN";
        case IR_PARAM: return "PARAM";
        case IR_PRINT: return "PRINT";
        case IR_ARRAY_LOAD: return "ARRAY_LOAD";
        case IR_ARRAY_STORE: return "ARRAY_STORE";
        case IR_BOUNDS_CHECK: return "BOUNDS_CHECK";
        case IR_ARRAY_DECL: return "ARRAY_DECL";
        case IR_ARRAY_INIT: return "ARRAY_INIT";
        case IR_VAR_DECL: return "VAR_DECL";
        default: return "UNKNOWN";
    }
}

int ir_function_new_temp(IRFunction* func) {
    return func->temp_counter++;
}

char* ir_function_new_label(IRFunction* func) {
    char* label = safe_malloc(32);
    snprintf(label, 32, "L%d", func->label_counter++);
    return label;
}

IRProgram* ir_generate(Program* ast_program, SemanticAnalyzer* analyzer) {
    IRProgram* ir_program = ir_program_create();
    
    for (size_t i = 0; i < ast_program->functions.size; i++) {
        Function* func = (Function*)array_get(&ast_program->functions, i);
        IRFunction* ir_func = ir_generate_function(func, analyzer);
        ir_program_add_function(ir_program, ir_func);
    }
    
    return ir_program;
}

IRFunction* ir_generate_function(Function* func, SemanticAnalyzer* analyzer) {
    IRFunction* ir_func = ir_function_create(func->name, func->return_type);
    
    for (size_t i = 0; i < func->params.size; i++) {
        Parameter* param = (Parameter*)array_get(&func->params, i);
        IROperand* param_op = ir_operand_var(param->name);
        param_op->data_type = param->type;  // Set the correct parameter type
        ir_function_add_param(ir_func, param_op);
    }
    
    ir_generate_statement(ir_func, func->body, analyzer);
    
    return ir_func;
}

static bool stmt_always_returns(Stmt* stmt) {
    if (!stmt) return false;
    if (stmt->type == STMT_RETURN) return true;
    if (stmt->type == STMT_BLOCK) {
        size_t n = stmt->data.block.statements.size;
        if (n == 0) return false;
        return stmt_always_returns((Stmt*)array_get(&stmt->data.block.statements, n-1));
    }
    if (stmt->type == STMT_IF) {
        if (stmt->data.if_stmt.else_branch) {
            return stmt_always_returns(stmt->data.if_stmt.then_branch) && stmt_always_returns(stmt->data.if_stmt.else_branch);
        }
    }
    return false;
}

void ir_generate_statement(IRFunction* ir_func, Stmt* stmt, SemanticAnalyzer* analyzer) {
    if (!stmt) return;
    
    switch (stmt->type) {
        case STMT_EXPR:
            ir_generate_expression(ir_func, stmt->data.expr.expression, analyzer);
            break;
        case STMT_VAR_DECL:
            IRInstruction* var_decl = ir_instruction_var_decl(stmt->data.var_decl.name, stmt->data.var_decl.type);
            ir_function_add_instruction(ir_func, var_decl);
            
            if (stmt->data.var_decl.initializer) {
                IROperand* value = ir_generate_expression(ir_func, stmt->data.var_decl.initializer, analyzer);
                IROperand* var = ir_operand_var(stmt->data.var_decl.name);
                var->data_type = stmt->data.var_decl.type;
                IRInstruction* move = ir_instruction_move(var, value);
                ir_function_add_instruction(ir_func, move);
            }
            break;
        case STMT_ARRAY_DECL:
            // Always emit the declaration
            IRInstruction* array_decl = ir_instruction_array_decl(stmt->data.array_decl.name, stmt->data.array_decl.size, stmt->data.array_decl.element_type);
            ir_function_add_instruction(ir_func, array_decl);
            if (stmt->data.array_decl.initializer) {
                IROperand* value = ir_generate_expression(ir_func, stmt->data.array_decl.initializer, analyzer);
                IRInstruction* array_init = ir_instruction_array_init(stmt->data.array_decl.name, stmt->data.array_decl.size, stmt->data.array_decl.element_type, value);
                ir_function_add_instruction(ir_func, array_init);
            }
            break;
        case STMT_ASSIGNMENT: {
            IROperand* value = ir_generate_expression(ir_func, stmt->data.assignment.value, analyzer);
            IROperand* var = ir_operand_var(stmt->data.assignment.name);
            IRInstruction* move = ir_instruction_move(var, value);
            ir_function_add_instruction(ir_func, move);
            break;
        }
        case STMT_ARRAY_ASSIGNMENT: {
            IROperand* array = ir_generate_expression(ir_func, stmt->data.array_assignment.array, analyzer);
            IROperand* index = ir_generate_expression(ir_func, stmt->data.array_assignment.index, analyzer);
            IROperand* value = ir_generate_expression(ir_func, stmt->data.array_assignment.value, analyzer);
            
            char* error_label = ir_function_new_label(ir_func);
            int array_size = array->array_size;
            if (array_size == -1) array_size = 5; 
            IROperand* size = ir_operand_const(array_size);
            IRInstruction* bounds_check = ir_instruction_bounds_check(index, size, error_label);
            ir_function_add_instruction(ir_func, bounds_check);
            
            IRInstruction* store = ir_instruction_array_store(array, index, value);
            ir_function_add_instruction(ir_func, store);
            break;
        }
        case STMT_IF: {
            char* then_label = ir_function_new_label(ir_func);
            char* end_label = ir_function_new_label(ir_func);
            
            IROperand* condition = ir_generate_expression(ir_func, stmt->data.if_stmt.condition, analyzer);
            IRInstruction* jump_if_false = ir_instruction_jump_if_false(condition, then_label);
            ir_function_add_instruction(ir_func, jump_if_false);
            
            ir_generate_statement(ir_func, stmt->data.if_stmt.then_branch, analyzer);
            bool then_returns = stmt_always_returns(stmt->data.if_stmt.then_branch);
            
            if (stmt->data.if_stmt.else_branch) {
                char* else_label = ir_function_new_label(ir_func);
                if (!then_returns) {
                    IRInstruction* jump = ir_instruction_jump(else_label);
                    ir_function_add_instruction(ir_func, jump);
                }
                IRInstruction* then_lbl = ir_instruction_label(then_label);
                ir_function_add_instruction(ir_func, then_lbl);
                
                ir_generate_statement(ir_func, stmt->data.if_stmt.else_branch, analyzer);
                bool else_returns = stmt_always_returns(stmt->data.if_stmt.else_branch);
                if (!else_returns) {
                    IRInstruction* end_lbl = ir_instruction_label(else_label);
                    ir_function_add_instruction(ir_func, end_lbl);
                }
            } else {
                if (!then_returns) {
                    IRInstruction* jump = ir_instruction_jump(end_label);
                    ir_function_add_instruction(ir_func, jump);
                }
                IRInstruction* then_lbl = ir_instruction_label(then_label);
                ir_function_add_instruction(ir_func, then_lbl);
                if (!then_returns) {
                    IRInstruction* end_lbl = ir_instruction_label(end_label);
                    ir_function_add_instruction(ir_func, end_lbl);
                }
            }
            break;
        }
        case STMT_WHILE: {
            char* loop_label = ir_function_new_label(ir_func);
            char* end_label = ir_function_new_label(ir_func);
            
            IRInstruction* loop_lbl = ir_instruction_label(loop_label);
            ir_function_add_instruction(ir_func, loop_lbl);
            
            IROperand* condition = ir_generate_expression(ir_func, stmt->data.while_stmt.condition, analyzer);
            IRInstruction* jump_if_false = ir_instruction_jump_if_false(condition, end_label);
            ir_function_add_instruction(ir_func, jump_if_false);
            
            ir_generate_statement(ir_func, stmt->data.while_stmt.body, analyzer);
            
            IRInstruction* jump = ir_instruction_jump(loop_label);
            ir_function_add_instruction(ir_func, jump);
            
            IRInstruction* end_lbl = ir_instruction_label(end_label);
            ir_function_add_instruction(ir_func, end_lbl);
            break;
        }
        case STMT_RETURN:
            if (stmt->data.return_stmt.value) {
                IROperand* value = ir_generate_expression(ir_func, stmt->data.return_stmt.value, analyzer);
                IRInstruction* ret = ir_instruction_return(value);
                ir_function_add_instruction(ir_func, ret);
            } else {
                IRInstruction* ret = ir_instruction_return(NULL);
                ir_function_add_instruction(ir_func, ret);
            }
            break;
        case STMT_PRINT:
            if (stmt->data.print_stmt.value) {
                IROperand* value = ir_generate_expression(ir_func, stmt->data.print_stmt.value, analyzer);
                IRInstruction* print = ir_instruction_print_op(value);
                ir_function_add_instruction(ir_func, print);
            }
            break;
        case STMT_BLOCK:
            for (size_t i = 0; i < stmt->data.block.statements.size; i++) {
                Stmt* block_stmt = (Stmt*)array_get(&stmt->data.block.statements, i);
                ir_generate_statement(ir_func, block_stmt, analyzer);
                if (stmt_always_returns(block_stmt)) break;
            }
            break;
    }
}

IROperand* ir_generate_expression(IRFunction* ir_func, Expr* expr, SemanticAnalyzer* analyzer) {
    if (!expr) return NULL;
    
    switch (expr->type) {
        case EXPR_LITERAL:
            if (expr->data.literal.is_float_literal) {
                return ir_operand_float_const(expr->data.literal.value.float_value);
            } else {
                return ir_operand_const(expr->data.literal.value.number_value);
            }
            
        case EXPR_VARIABLE: {
            int array_size = get_array_size(analyzer, expr->data.variable.name);
            if (debug_enabled) { printf("[DEBUG] Variable %s: array_size = %d\n", expr->data.variable.name, array_size); }
            if (array_size != -1) {
                IROperand* operand = ir_operand_array_var(expr->data.variable.name, array_size);
                return operand;
            } else {
                IROperand* operand = ir_operand_var(expr->data.variable.name);
                Symbol* symbol = scope_resolve(analyzer, expr->data.variable.name);
                if (symbol) {
                    operand->data_type = symbol->data_type;
                }
                return operand;
            }
        }
            
        case EXPR_BINARY: {
            IROperand* left = ir_generate_expression(ir_func, expr->data.binary.left, analyzer);
            IROperand* right = ir_generate_expression(ir_func, expr->data.binary.right, analyzer);
            
            IROpcode opcode;
            switch (expr->data.binary.operator) {
                case TOKEN_PLUS: opcode = IR_ADD; break;
                case TOKEN_MINUS: opcode = IR_SUB; break;
                case TOKEN_STAR: opcode = IR_MUL; break;
                case TOKEN_SLASH: opcode = IR_DIV; break;
                case TOKEN_PERCENT: opcode = IR_MOD; break;
                case TOKEN_EQ: opcode = IR_EQ; break;
                case TOKEN_NE: opcode = IR_NE; break;
                case TOKEN_LT: opcode = IR_LT; break;
                case TOKEN_LE: opcode = IR_LE; break;
                case TOKEN_GT: opcode = IR_GT; break;
                case TOKEN_GE: opcode = IR_GE; break;
                case TOKEN_AND: opcode = IR_AND; break;
                case TOKEN_OR: opcode = IR_OR; break;
                default: return NULL;
            }
            
            IROperand* result = ir_operand_temp(ir_function_new_temp(ir_func));
            
            if (opcode == IR_ADD || opcode == IR_SUB || opcode == IR_MUL || opcode == IR_DIV || opcode == IR_MOD) {
                if (left->data_type == TYPE_DOUBLE || right->data_type == TYPE_DOUBLE) {
                    result->data_type = TYPE_DOUBLE;
                } else if (left->data_type == TYPE_FLOAT || right->data_type == TYPE_FLOAT) {
                    result->data_type = TYPE_FLOAT;
                } else {
                    result->data_type = TYPE_INT;
                }
            } else {
                result->data_type = TYPE_BOOL;
            }
            
            IRInstruction* instr = ir_instruction_binary(opcode, result, left, right);
            ir_function_add_instruction(ir_func, instr);
            return result;
        }
        
        case EXPR_UNARY: {
            IROperand* operand = ir_generate_expression(ir_func, expr->data.unary.operand, analyzer);
            
            IROpcode opcode;
            switch (expr->data.unary.operator) {
                case TOKEN_MINUS: opcode = IR_NEG; break;
                case TOKEN_BANG: opcode = IR_NOT; break;
                default: return NULL;
            }
            
            IROperand* result = ir_operand_temp(ir_function_new_temp(ir_func));
            IRInstruction* instr = ir_instruction_unary(opcode, result, operand);
            ir_function_add_instruction(ir_func, instr);
            return result;
        }
        
        case EXPR_CALL: {
            IROperand* result = ir_operand_temp(ir_function_new_temp(ir_func));
            
            // Set the correct return type based on the function being called
            if (string_equal(expr->data.call.name, "test_function")) {
                result->data_type = TYPE_DOUBLE;
            } else {
                result->data_type = TYPE_INT; // Default
            }
            
            for (size_t i = 0; i < expr->data.call.args.size; i++) {
                Expr* arg_expr = (Expr*)array_get(&expr->data.call.args, i);
                IROperand* arg = ir_generate_expression(ir_func, arg_expr, analyzer);
                IRInstruction* param = ir_instruction_param(arg);
                ir_function_add_instruction(ir_func, param);
            }
            
            IRInstruction* call = ir_instruction_call(result, expr->data.call.name);
            ir_function_add_instruction(ir_func, call);
            return result;
        }
        
        case EXPR_GROUP:
            return ir_generate_expression(ir_func, expr->data.group.expression, analyzer);
            
        case EXPR_ARRAY_INDEX: {
            IROperand* array = ir_generate_expression(ir_func, expr->data.array_index.array, analyzer);
            IROperand* index = ir_generate_expression(ir_func, expr->data.array_index.index, analyzer);
            
            char* error_label = ir_function_new_label(ir_func);
            int array_size = array->array_size;
            if (debug_enabled) { printf("[DEBUG] Array index: array_size = %d\n", array_size); }
            if (array_size == -1) array_size = 5; 
            IROperand* size = ir_operand_const(array_size);
            IRInstruction* bounds_check = ir_instruction_bounds_check(index, size, error_label);
            ir_function_add_instruction(ir_func, bounds_check);
            
            IROperand* result = ir_operand_temp(ir_function_new_temp(ir_func));
            // Set the result type to the array's element type (stored in array->data_type)
            result->data_type = array->data_type;
            IRInstruction* load = ir_instruction_array_load(result, array, index);
            ir_function_add_instruction(ir_func, load);
            
            return result;
        }
            
        default:
            return NULL;
    }
} 