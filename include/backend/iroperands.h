#ifndef IROPERANDS_H
#define IROPERANDS_H

#include "common.h"
#include "backend/ir.h"
#include "backend/codegen.h"

IROperand *ir_operand_temp(int temp_id);
IROperand *ir_operand_var(const char *var_name);
IROperand *ir_operand_array_var(const char *var_name, int size);
IROperand *ir_operand_const(int64_t value);
IROperand *ir_operand_float_const(double value);
IROperand *ir_operand_string_const(const char *value);
IROperand *ir_operand_null(void);
IROperand *ir_operand_null_with_type(DataType data_type);
IROperand *ir_operand_label(const char *label_name);

void ir_operand_destroy(IROperand *operand);
void ir_operand_print(const IROperand *operand);

void codegen_write_operand(CodeGenerator *generator, IROperand *operand);
char *codegen_get_temp_name(CodeGenerator *generator, IROperand *operand);

char *codegenasm_get_operand_name(CodeGenerator *generator, IROperand *operand);
char *codegenasm_get_temp_name(CodeGenerator *generator, IROperand *operand);
char *codegenasm_get_const_name(CodeGenerator *generator, IROperand *operand);

#endif