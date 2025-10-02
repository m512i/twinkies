#include "backend/iroperands.h"

static void escape_string_for_c(const char *input, FILE *output) {
    while (*input) {
        switch (*input) {
            case '\n':
                fprintf(output, "\\n");
                break;
            case '\t':
                fprintf(output, "\\t");
                break;
            case '\r':
                fprintf(output, "\\r");
                break;
            case '\\':
                fprintf(output, "\\\\");
                break;
            case '"':
                fprintf(output, "\\\"");
                break;
            default:
                fputc(*input, output);
                break;
        }
        input++;
    }
}

extern bool debug_enabled;

IROperand *ir_operand_temp(int temp_id)
{
    IROperand *operand = safe_malloc(sizeof(IROperand));
    operand->type = IR_OP_TEMP;
    operand->array_size = -1;
    operand->is_float_const = false;
    operand->data_type = TYPE_INT;
    operand->data.temp_id = temp_id;
    return operand;
}

IROperand *ir_operand_var(const char *var_name)
{
    IROperand *operand = safe_malloc(sizeof(IROperand));
    operand->type = IR_OP_VAR;
    operand->array_size = -1;
    operand->is_float_const = false;
    operand->data_type = TYPE_INT;
    operand->data.var_name = string_copy(var_name);
    return operand;
}

IROperand *ir_operand_array_var(const char *var_name, int size)
{
    IROperand *operand = safe_malloc(sizeof(IROperand));
    operand->type = IR_OP_VAR;
    operand->array_size = size;
    operand->is_float_const = false;
    operand->data_type = TYPE_ARRAY;
    operand->data.var_name = string_copy(var_name);
    return operand;
}

IROperand *ir_operand_const(int64_t value)
{
    IROperand *operand = safe_malloc(sizeof(IROperand));
    operand->type = IR_OP_CONST;
    operand->array_size = -1;
    operand->is_float_const = false;
    operand->data_type = TYPE_INT;
    operand->data.const_value = value;
    return operand;
}

IROperand *ir_operand_float_const(double value)
{
    IROperand *operand = safe_malloc(sizeof(IROperand));
    operand->type = IR_OP_CONST;
    operand->array_size = -1;
    operand->is_float_const = true;
    operand->data_type = TYPE_FLOAT;
    operand->data.float_const_value = value;
    return operand;
}

IROperand *ir_operand_string_const(const char *value)
{
    IROperand *operand = safe_malloc(sizeof(IROperand));
    operand->type = IR_OP_STRING_CONST;
    operand->array_size = -1;
    operand->is_float_const = false;
    operand->data_type = TYPE_STRING;
    operand->data.string_const_value = string_copy(value);
    return operand;
}

IROperand *ir_operand_null(void)
{
    IROperand *operand = safe_malloc(sizeof(IROperand));
    operand->type = IR_OP_NULL;
    operand->array_size = -1;
    operand->is_float_const = false;
    operand->data_type = TYPE_NULL;
    return operand;
}

IROperand *ir_operand_null_with_type(DataType data_type)
{
    IROperand *operand = safe_malloc(sizeof(IROperand));
    operand->type = IR_OP_NULL;
    operand->array_size = -1;
    operand->is_float_const = false;
    operand->data_type = data_type;
    return operand;
}

IROperand *ir_operand_label(const char *label_name)
{
    IROperand *operand = safe_malloc(sizeof(IROperand));
    operand->type = IR_OP_LABEL;
    operand->array_size = -1;
    operand->is_float_const = false;
    operand->data_type = TYPE_VOID;
    operand->data.label_name = string_copy(label_name);
    return operand;
}

void ir_operand_destroy(IROperand *operand)
{
    if (!operand)
        return;

    switch (operand->type)
    {
    case IR_OP_VAR:
        safe_free(operand->data.var_name);
        break;
    case IR_OP_STRING_CONST:
        safe_free(operand->data.string_const_value);
        break;
    case IR_OP_LABEL:
        safe_free(operand->data.label_name);
        break;
    default:
        break;
    }
    safe_free(operand);
}

void ir_operand_print(const IROperand *operand)
{
    if (!operand)
    {
        printf("NULL");
        return;
    }

    switch (operand->type)
    {
    case IR_OP_TEMP:
        printf("t%d", operand->data.temp_id);
        break;
    case IR_OP_VAR:
        printf("%s", operand->data.var_name);
        break;
    case IR_OP_CONST:
        if (operand->is_float_const)
        {
            double float_value = (double)operand->data.const_value / 1000000.0;
            printf("%f", float_value);
        }
        else
        {
            printf("%lld", operand->data.const_value);
        }
        break;
    case IR_OP_STRING_CONST:
        printf("\"%s\"", operand->data.string_const_value);
        break;
    case IR_OP_LABEL:
        printf("%s", operand->data.label_name);
        break;
    case IR_OP_NULL:
        printf("NULL");
        break;
    }
}

char *codegen_get_temp_name(CodeGenerator *generator, IROperand *operand)
{
    (void)generator;
    if (!operand || operand->type != IR_OP_TEMP)
        return NULL;

    char *new_name = safe_malloc(32);
    snprintf(new_name, 32, "temp_%d", operand->data.temp_id);
    return new_name;
}

char *codegenasm_get_operand_name(CodeGenerator *generator, IROperand *operand)
{
    if (!operand)
        return "0";
    if (operand->type == IR_OP_VAR && generator->current_function_name)
    {
        IRFunction *func = NULL;
        for (size_t i = 0; i < generator->ir_program->functions.size; i++)
        {
            IRFunction *f = (IRFunction *)array_get(&generator->ir_program->functions, i);
            if (string_equal(f->name, generator->current_function_name))
            {
                func = f;
                break;
            }
        }
        if (func)
        {
            for (size_t i = 0; i < func->params.size && i < 4; i++)
            {
                IROperand *param = (IROperand *)array_get(&func->params, i);
                if (string_equal(param->data.var_name, operand->data.var_name))
                {
                    static char *regs[] = {"rcx", "rdx", "r8", "r9"};
                    return regs[i];
                }
            }
        }
    }
    return operand->data.var_name;
}

char *codegenasm_get_temp_name(CodeGenerator *generator, IROperand *operand)
{
    (void)generator;
    if (!operand || operand->type != IR_OP_TEMP)
        return "0";
    char *name = safe_malloc(32);
    snprintf(name, 32, "temp_%d", operand->data.temp_id);
    return name;
}

char *codegenasm_get_const_name(CodeGenerator *generator, IROperand *operand)
{
    (void)generator;
    if (!operand || operand->type != IR_OP_CONST)
        return "0";
    char *name = safe_malloc(32);
    if (operand->is_float_const)
    {
        double value = (double)operand->data.const_value / 1000000.0;
        snprintf(name, 32, "%f", value);
    }
    else
    {
        snprintf(name, 32, "%lld", operand->data.const_value);
    }
    return name;
}