#include "../include/codegen.h"
#include <stdarg.h>

CodeGenerator* codegen_create(IRProgram* ir_program, FILE* output_file, Error* error) {
    CodeGenerator* generator = safe_malloc(sizeof(CodeGenerator));
    generator->ir_program = ir_program;
    generator->output_file = output_file;
    generator->error = error;
    generator->indent_level = 0;
    generator->temp_counter = 0;
    generator->temp_map = hashtable_create(16);
    generator->var_set = hashtable_create(16); 
    generator->array_info = hashtable_create(16);
    generator->param_count = 0; 
    return generator;
}

void codegen_destroy(CodeGenerator* generator) {
    if (!generator) return;
    
    hashtable_destroy(generator->temp_map);
    hashtable_destroy(generator->var_set);
    hashtable_destroy(generator->array_info);
    safe_free(generator);
}

bool codegen_generate(CodeGenerator* generator) {
    codegen_write_header(generator);
    codegen_generate_program(generator);
    return true;
}

void codegen_generate_program(CodeGenerator* generator) {
    for (size_t i = 0; i < generator->ir_program->functions.size; i++) {
        IRFunction* func = (IRFunction*)array_get(&generator->ir_program->functions, i);
        codegen_generate_function(generator, func);
    }
    
    bool has_main = false;
    for (size_t i = 0; i < generator->ir_program->functions.size; i++) {
        IRFunction* func = (IRFunction*)array_get(&generator->ir_program->functions, i);
        if (string_equal(func->name, "main")) {
            has_main = true;
            break;
        }
    }
    
    if (!has_main) {
        codegen_write_main_function(generator);
    }
}

void codegen_generate_function(CodeGenerator* generator, IRFunction* func) {
    codegen_write_function_header(generator, func);
    
    for (size_t i = 0; i < func->instructions.size; i++) {
        IRInstruction* instr = (IRInstruction*)array_get(&func->instructions, i);
        codegen_generate_instruction(generator, instr);
    }
    
    codegen_write_function_footer(generator);
}

void codegen_generate_instruction(CodeGenerator* generator, IRInstruction* instr) {
    if (!instr) return;
    
    switch (instr->opcode) {
        case IR_NOP:
            break;
            
        case IR_LABEL:
            codegen_write_line(generator, "%s:", instr->label);
            break;
            
        case IR_MOVE:
            codegen_write_indent(generator);
            codegen_write_operand(generator, instr->result);
            fprintf(generator->output_file, " = ");
            codegen_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, ";\n");
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
            codegen_write_indent(generator);
            codegen_write_operand(generator, instr->result);
            fprintf(generator->output_file, " = ");
            codegen_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, " %s ", ir_opcode_to_string(instr->opcode));
            codegen_write_operand(generator, instr->arg2);
            fprintf(generator->output_file, ";\n");
            break;
            
        case IR_NEG:
        case IR_NOT:
            codegen_write_indent(generator);
            codegen_write_operand(generator, instr->result);
            fprintf(generator->output_file, " = %s", 
                   instr->opcode == IR_NEG ? "-" : "!");
            codegen_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, ";\n");
            break;
            
        case IR_JUMP:
            codegen_write_indent(generator);
            fprintf(generator->output_file, "goto %s;\n", instr->label);
            break;
            
        case IR_JUMP_IF:
            codegen_write_indent(generator);
            fprintf(generator->output_file, "if (");
            codegen_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, ") goto %s;\n", instr->label);
            break;
            
        case IR_JUMP_IF_FALSE:
            codegen_write_indent(generator);
            fprintf(generator->output_file, "if (!");
            codegen_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, ") goto %s;\n", instr->label);
            break;
            
        case IR_PARAM:
            if (generator->param_count < MAX_PARAMS) {
                generator->params[generator->param_count++] = instr->arg1;
            }
            break;
            
        case IR_CALL:
            codegen_write_indent(generator);
            if (instr->result) {
                codegen_write_operand(generator, instr->result);
                fprintf(generator->output_file, " = ");
            }
            fprintf(generator->output_file, "%s(", instr->label);
            
            for (int i = 0; i < generator->param_count; i++) {
                if (i > 0) fprintf(generator->output_file, ", ");
                codegen_write_operand(generator, generator->params[i]);
            }
            
            fprintf(generator->output_file, ");\n");
            generator->param_count = 0;
            break;
            
        case IR_RETURN:
            codegen_write_indent(generator);
            if (instr->arg1) {
                fprintf(generator->output_file, "return ");
                codegen_write_operand(generator, instr->arg1);
                fprintf(generator->output_file, ";\n");
            } else {
                fprintf(generator->output_file, "return 0;\n");
            }
            break;
            
        case IR_PRINT:
            codegen_write_indent(generator);
            fprintf(generator->output_file, "printf(\"%%\" PRId64 \"\\n\", ");
            codegen_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, ");\n");
            break;
        case IR_ARRAY_LOAD:
            codegen_write_indent(generator);
            codegen_write_operand(generator, instr->result);
            fprintf(generator->output_file, " = ");
            codegen_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, "[");
            codegen_write_operand(generator, instr->arg2);
            fprintf(generator->output_file, "];\n");
            break;
        case IR_ARRAY_STORE:
            codegen_write_indent(generator);
            codegen_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, "[");
            codegen_write_operand(generator, instr->arg2);
            fprintf(generator->output_file, "] = ");
            codegen_write_operand(generator, instr->result);
            fprintf(generator->output_file, ";\n");
            break;
        case IR_BOUNDS_CHECK:
            codegen_write_indent(generator);
            fprintf(generator->output_file, "if (");
            codegen_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, " >= ");
            codegen_write_operand(generator, instr->arg2);
            fprintf(generator->output_file, ") {\n");
            generator->indent_level++;
            codegen_write_indent(generator);
            fprintf(generator->output_file, "fprintf(stderr, \"Array index out of bounds\\n\");\n");
            codegen_write_indent(generator);
            fprintf(generator->output_file, "exit(1);\n");
            generator->indent_level--;
            codegen_write_indent(generator);
            fprintf(generator->output_file, "}\n");
            break;
        case IR_ARRAY_DECL:
            // Array declarations are handled in function header generation
            break;
        case IR_ARRAY_INIT:
            // Generate a loop to initialize all array elements
            codegen_write_indent(generator);
            fprintf(generator->output_file, "for (int i = 0; i < %d; i++) {\n", instr->result->array_size);
            generator->indent_level++;
            codegen_write_indent(generator);
            codegen_write_operand(generator, instr->result);
            fprintf(generator->output_file, "[i] = ");
            codegen_write_operand(generator, instr->arg1);
            fprintf(generator->output_file, ";\n");
            generator->indent_level--;
            codegen_write_indent(generator);
            fprintf(generator->output_file, "}\n");
            break;
    }
}

void codegen_write_indent(CodeGenerator* generator) {
    for (int i = 0; i < generator->indent_level; i++) {
        fprintf(generator->output_file, "    ");
    }
}

void codegen_write_line(CodeGenerator* generator, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    codegen_write_indent(generator);
    vfprintf(generator->output_file, format, args);
    fprintf(generator->output_file, "\n");
    
    va_end(args);
}

void codegen_write_operand(CodeGenerator* generator, IROperand* operand) {
    if (!operand) {
        fprintf(generator->output_file, "0");
        return;
    }
    
    switch (operand->type) {
        case IR_OP_TEMP: {
            char temp_name[32];
            snprintf(temp_name, sizeof(temp_name), "temp_%d", operand->data.temp_id);
            fprintf(generator->output_file, "%s", temp_name);
            break;
        }
        case IR_OP_VAR:
            fprintf(generator->output_file, "%s", operand->data.var_name);
            break;
        case IR_OP_CONST:
            fprintf(generator->output_file, "%lld", operand->data.const_value);
            break;
        case IR_OP_LABEL:
            fprintf(generator->output_file, "%s", operand->data.label_name);
            break;
    }
}

char* codegen_get_temp_name(CodeGenerator* generator, IROperand* operand) {
    if (operand->type != IR_OP_TEMP) return NULL;
    
    char temp_key[32];
    snprintf(temp_key, sizeof(temp_key), "t%d", operand->data.temp_id);
    
    char* existing_name = hashtable_get(generator->temp_map, temp_key);
    if (existing_name) {
        return existing_name;
    }
    
    char* new_name = safe_malloc(32);
    snprintf(new_name, 32, "temp_%d", generator->temp_counter++);
    hashtable_put(generator->temp_map, temp_key, new_name);
    
    return new_name;
}

void codegen_write_header(CodeGenerator* generator) {
    fprintf(generator->output_file, "#include <stdio.h>\n");
    fprintf(generator->output_file, "#include <stdlib.h>\n");
    fprintf(generator->output_file, "#include <stdint.h>\n");
    fprintf(generator->output_file, "#include <stdbool.h>\n");
    fprintf(generator->output_file, "#include <inttypes.h>\n");
    fprintf(generator->output_file, "\n");
}

bool codegen_is_array_variable(CodeGenerator* generator, const char* var_name) {
    for (size_t i = 0; i < generator->ir_program->functions.size; i++) {
        IRFunction* func = (IRFunction*)array_get(&generator->ir_program->functions, i);
        for (size_t j = 0; j < func->instructions.size; j++) {
            IRInstruction* instr = (IRInstruction*)array_get(&func->instructions, j);
            if (instr->opcode == IR_ARRAY_DECL) {
                if (instr->result && instr->result->type == IR_OP_VAR && 
                    string_equal(instr->result->data.var_name, var_name)) {
                    return true;
                }
            }
            if (instr->opcode == IR_ARRAY_LOAD || instr->opcode == IR_ARRAY_STORE) {
                if (instr->arg1 && instr->arg1->type == IR_OP_VAR && 
                    string_equal(instr->arg1->data.var_name, var_name)) {
                    return true;
                }
            }
        }
    }
    return false;
}

int codegen_get_array_size(CodeGenerator* generator, const char* var_name) {
    for (size_t i = 0; i < generator->ir_program->functions.size; i++) {
        IRFunction* func = (IRFunction*)array_get(&generator->ir_program->functions, i);
        for (size_t j = 0; j < func->instructions.size; j++) {
            IRInstruction* instr = (IRInstruction*)array_get(&func->instructions, j);
            if (instr->opcode == IR_ARRAY_DECL) {
                if (instr->result && instr->result->type == IR_OP_VAR && 
                    string_equal(instr->result->data.var_name, var_name)) {
                    return instr->result->array_size;
                }
            }
            if (instr->opcode == IR_ARRAY_LOAD || instr->opcode == IR_ARRAY_STORE) {
                if (instr->arg1 && instr->arg1->type == IR_OP_VAR && 
                    string_equal(instr->arg1->data.var_name, var_name)) {
                    return instr->arg1->array_size;
                }
            }
        }
    }
    return -1; // Not found
}

void codegen_write_function_header(CodeGenerator* generator, IRFunction* func) {
    if (string_equal(func->name, "main")) {
        fprintf(generator->output_file, "int main(void) {\n");
    } else {
        fprintf(generator->output_file, "int64_t %s(", func->name);
        
        if (func->params.size == 0) {
            fprintf(generator->output_file, "void");
        } else {
            for (size_t i = 0; i < func->params.size; i++) {
                if (i > 0) fprintf(generator->output_file, ", ");
                IROperand* param = (IROperand*)array_get(&func->params, i);
                fprintf(generator->output_file, "int64_t %s", param->data.var_name);
            }
        }
        
        fprintf(generator->output_file, ") {\n");
    }
    
    generator->indent_level++;
    
    for (size_t i = 0; i < func->instructions.size; i++) {
        IRInstruction* instr = (IRInstruction*)array_get(&func->instructions, i);
        
        if (instr->opcode == IR_ARRAY_DECL && instr->result && instr->result->type == IR_OP_VAR) {
            char* var_name = instr->result->data.var_name;
            bool is_param = false;
            for (size_t j = 0; j < func->params.size; j++) {
                IROperand* param = (IROperand*)array_get(&func->params, j);
                if (string_equal(var_name, param->data.var_name)) {
                    is_param = true;
                    break;
                }
            }
            if (!is_param && !hashtable_get(generator->var_set, var_name)) {
                hashtable_put(generator->var_set, var_name, (void*)1);
                int array_size = instr->result->array_size;
                if (array_size == -1) array_size = 5; // Fallback
                codegen_write_line(generator, "int64_t %s[%d];", var_name, array_size);
            }
        }
        
        if (instr->result && instr->result->type == IR_OP_VAR) {
            char* var_name = instr->result->data.var_name;
            bool is_param = false;
            for (size_t j = 0; j < func->params.size; j++) {
                IROperand* param = (IROperand*)array_get(&func->params, j);
                if (string_equal(var_name, param->data.var_name)) {
                    is_param = true;
                    break;
                }
            }
            if (!is_param && !hashtable_get(generator->var_set, var_name)) {
                hashtable_put(generator->var_set, var_name, (void*)1);
                if (codegen_is_array_variable(generator, var_name)) {
                    int array_size = codegen_get_array_size(generator, var_name);
                    if (array_size == -1) array_size = 5; // Fallback
                    codegen_write_line(generator, "int64_t %s[%d];", var_name, array_size);
                } else {
                    codegen_write_line(generator, "int64_t %s;", var_name);
                }
            }
        }
        
        if (instr->arg1 && instr->arg1->type == IR_OP_VAR) {
            char* var_name = instr->arg1->data.var_name;
            bool is_param = false;
            for (size_t j = 0; j < func->params.size; j++) {
                IROperand* param = (IROperand*)array_get(&func->params, j);
                if (string_equal(var_name, param->data.var_name)) {
                    is_param = true;
                    break;
                }
            }
            if (!is_param && !hashtable_get(generator->var_set, var_name)) {
                hashtable_put(generator->var_set, var_name, (void*)1);
                if (codegen_is_array_variable(generator, var_name)) {
                    int array_size = codegen_get_array_size(generator, var_name);
                    if (array_size == -1) array_size = 5; // Fallback
                    codegen_write_line(generator, "int64_t %s[%d];", var_name, array_size);
                } else {
                    codegen_write_line(generator, "int64_t %s;", var_name);
                }
            }
        }
        
        if (instr->arg2 && instr->arg2->type == IR_OP_VAR) {
            char* var_name = instr->arg2->data.var_name;
            bool is_param = false;
            for (size_t j = 0; j < func->params.size; j++) {
                IROperand* param = (IROperand*)array_get(&func->params, j);
                if (string_equal(var_name, param->data.var_name)) {
                    is_param = true;
                    break;
                }
            }
            if (!is_param && !hashtable_get(generator->var_set, var_name)) {
                hashtable_put(generator->var_set, var_name, (void*)1);
                if (codegen_is_array_variable(generator, var_name)) {
                    int array_size = codegen_get_array_size(generator, var_name);
                    if (array_size == -1) array_size = 5; // Fallback
                    codegen_write_line(generator, "int64_t %s[%d];", var_name, array_size);
                } else {
                    codegen_write_line(generator, "int64_t %s;", var_name);
                }
            }
        }
    }
    
    for (int i = 0; i < func->temp_counter; i++) {
        char temp_name[32];
        snprintf(temp_name, sizeof(temp_name), "temp_%d", i);
        codegen_write_line(generator, "int64_t %s;", temp_name);
    }
    
    if (func->temp_counter > 0 || generator->var_set->size > 0) {
        fprintf(generator->output_file, "\n");
    }
}

void codegen_write_function_footer(CodeGenerator* generator) {
    generator->indent_level--;
    fprintf(generator->output_file, "}\n\n");
}

void codegen_write_main_function(CodeGenerator* generator) {
    fprintf(generator->output_file, "int main() {\n");
    fprintf(generator->output_file, "    return 0;\n");
    fprintf(generator->output_file, "}\n");
}

void codegen_error(CodeGenerator* generator, const char* message) {
    if (generator->error) {
        error_set(generator->error, ERROR_CODEGEN, message, 0, 0);
    }
} 