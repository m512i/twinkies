#include "../include/ast.h"

Expr* expr_literal_number(int64_t value, int line, int column) {
    Expr* expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->line = line;
    expr->column = column;
    expr->data.literal.value.number_value = value;
    expr->data.literal.is_bool_literal = false;
    expr->data.literal.is_float_literal = false;
    return expr;
}

Expr* expr_literal_bool(bool value, int line, int column) {
    Expr* expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->line = line;
    expr->column = column;
    expr->data.literal.value.bool_value = value;
    expr->data.literal.is_bool_literal = true;
    expr->data.literal.is_float_literal = false;
    return expr;
}

Expr* expr_literal_float(double value, int line, int column) {
    Expr* expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->line = line;
    expr->column = column;
    expr->data.literal.value.float_value = value;
    expr->data.literal.is_bool_literal = false;
    expr->data.literal.is_float_literal = true;
    return expr;
}

Expr* expr_variable(const char* name, int line, int column) {
    Expr* expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_VARIABLE;
    expr->line = line;
    expr->column = column;
    expr->data.variable.name = string_copy(name);
    return expr;
}

Expr* expr_binary(Expr* left, TLTokenType operator, Expr* right, int line, int column) {
    Expr* expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_BINARY;
    expr->line = line;
    expr->column = column;
    expr->data.binary.left = left;
    expr->data.binary.operator = operator;
    expr->data.binary.right = right;
    return expr;
}

Expr* expr_unary(TLTokenType operator, Expr* operand, int line, int column) {
    Expr* expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_UNARY;
    expr->line = line;
    expr->column = column;
    expr->data.unary.operator = operator;
    expr->data.unary.operand = operand;
    return expr;
}

Expr* expr_call(const char* name, int line, int column) {
    Expr* expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_CALL;
    expr->line = line;
    expr->column = column;
    expr->data.call.name = string_copy(name);
    array_init(&expr->data.call.args, 4);
    return expr;
}

Expr* expr_group(Expr* expression, int line, int column) {
    Expr* expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_GROUP;
    expr->line = line;
    expr->column = column;
    expr->data.group.expression = expression;
    return expr;
}

Expr* expr_array_index(Expr* array, Expr* index, int line, int column) {
    Expr* expr = safe_malloc(sizeof(Expr));
    expr->type = EXPR_ARRAY_INDEX;
    expr->line = line;
    expr->column = column;
    expr->data.array_index.array = array;
    expr->data.array_index.index = index;
    return expr;
}

Stmt* stmt_expr(Expr* expression, int line, int column) {
    Stmt* stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_EXPR;
    stmt->line = line;
    stmt->column = column;
    stmt->data.expr.expression = expression;
    return stmt;
}

Stmt* stmt_var_decl(const char* name, DataType type, Expr* initializer, int line, int column) {
    Stmt* stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_VAR_DECL;
    stmt->line = line;
    stmt->column = column;
    stmt->data.var_decl.name = string_copy(name);
    stmt->data.var_decl.type = type;
    stmt->data.var_decl.initializer = initializer;
    return stmt;
}

Stmt* stmt_array_decl(const char* name, DataType element_type, int size, Expr* initializer, int line, int column) {
    Stmt* stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_ARRAY_DECL;
    stmt->line = line;
    stmt->column = column;
    stmt->data.array_decl.name = string_copy(name);
    stmt->data.array_decl.element_type = element_type;
    stmt->data.array_decl.size = size;
    stmt->data.array_decl.initializer = initializer;
    return stmt;
}

Stmt* stmt_assignment(const char* name, Expr* value, int line, int column) {
    Stmt* stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_ASSIGNMENT;
    stmt->line = line;
    stmt->column = column;
    stmt->data.assignment.name = string_copy(name);
    stmt->data.assignment.value = value;
    return stmt;
}

Stmt* stmt_array_assignment(Expr* array, Expr* index, Expr* value, int line, int column) {
    Stmt* stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_ARRAY_ASSIGNMENT;
    stmt->line = line;
    stmt->column = column;
    stmt->data.array_assignment.array = array;
    stmt->data.array_assignment.index = index;
    stmt->data.array_assignment.value = value;
    return stmt;
}

Stmt* stmt_if(Expr* condition, Stmt* then_branch, Stmt* else_branch, int line, int column) {
    Stmt* stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_IF;
    stmt->line = line;
    stmt->column = column;
    stmt->data.if_stmt.condition = condition;
    stmt->data.if_stmt.then_branch = then_branch;
    stmt->data.if_stmt.else_branch = else_branch;
    return stmt;
}

Stmt* stmt_while(Expr* condition, Stmt* body, int line, int column) {
    Stmt* stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_WHILE;
    stmt->line = line;
    stmt->column = column;
    stmt->data.while_stmt.condition = condition;
    stmt->data.while_stmt.body = body;
    return stmt;
}

Stmt* stmt_return(Expr* value, int line, int column) {
    Stmt* stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_RETURN;
    stmt->line = line;
    stmt->column = column;
    stmt->data.return_stmt.value = value;
    return stmt;
}

Stmt* stmt_print_stmt(Expr* value, int line, int column) {
    Stmt* stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_PRINT;
    stmt->line = line;
    stmt->column = column;
    stmt->data.print_stmt.value = value;
    return stmt;
}

Stmt* stmt_block(int line, int column) {
    Stmt* stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_BLOCK;
    stmt->line = line;
    stmt->column = column;
    array_init(&stmt->data.block.statements, 8);
    return stmt;
}

Function* function_create(const char* name, DataType return_type) {
    Function* func = safe_malloc(sizeof(Function));
    func->name = string_copy(name);
    func->return_type = return_type;
    array_init(&func->params, 4);
    func->body = NULL;
    return func;
}

Parameter* parameter_create(const char* name, DataType type) {
    Parameter* param = safe_malloc(sizeof(Parameter));
    param->name = string_copy(name);
    param->type = type;
    return param;
}

Program* program_create(void) {
    Program* program = safe_malloc(sizeof(Program));
    array_init(&program->functions, 4);
    return program;
}

void expr_add_call_arg(Expr* call, Expr* arg) {
    array_push(&call->data.call.args, arg);
}

void stmt_add_block_stmt(Stmt* block, Stmt* stmt) {
    array_push(&block->data.block.statements, stmt);
}

void function_add_param(Function* func, Parameter* param) {
    array_push(&func->params, param);
}

void program_add_function(Program* program, Function* func) {
    array_push(&program->functions, func);
}

void expr_destroy(Expr* expr) {
    if (!expr) return;
    
    switch (expr->type) {
        case EXPR_VARIABLE:
            safe_free(expr->data.variable.name);
            break;
        case EXPR_BINARY:
            expr_destroy(expr->data.binary.left);
            expr_destroy(expr->data.binary.right);
            break;
        case EXPR_UNARY:
            expr_destroy(expr->data.unary.operand);
            break;
        case EXPR_CALL:
            safe_free(expr->data.call.name);
            for (size_t i = 0; i < expr->data.call.args.size; i++) {
                expr_destroy((Expr*)array_get(&expr->data.call.args, i));
            }
            array_free(&expr->data.call.args);
            break;
        case EXPR_GROUP:
            expr_destroy(expr->data.group.expression);
            break;
        case EXPR_ARRAY_INDEX:
            expr_destroy(expr->data.array_index.array);
            expr_destroy(expr->data.array_index.index);
            break;
        default:
            break;
    }
    safe_free(expr);
}

void stmt_destroy(Stmt* stmt) {
    if (!stmt) return;
    
    switch (stmt->type) {
        case STMT_EXPR:
            expr_destroy(stmt->data.expr.expression);
            break;
        case STMT_VAR_DECL:
            safe_free(stmt->data.var_decl.name);
            expr_destroy(stmt->data.var_decl.initializer);
            break;
        case STMT_ARRAY_DECL:
            safe_free(stmt->data.array_decl.name);
            expr_destroy(stmt->data.array_decl.initializer);
            break;
        case STMT_ASSIGNMENT:
            safe_free(stmt->data.assignment.name);
            expr_destroy(stmt->data.assignment.value);
            break;
        case STMT_ARRAY_ASSIGNMENT:
            expr_destroy(stmt->data.array_assignment.array);
            expr_destroy(stmt->data.array_assignment.index);
            expr_destroy(stmt->data.array_assignment.value);
            break;
        case STMT_IF:
            expr_destroy(stmt->data.if_stmt.condition);
            stmt_destroy(stmt->data.if_stmt.then_branch);
            stmt_destroy(stmt->data.if_stmt.else_branch);
            break;
        case STMT_WHILE:
            expr_destroy(stmt->data.while_stmt.condition);
            stmt_destroy(stmt->data.while_stmt.body);
            break;
        case STMT_RETURN:
            expr_destroy(stmt->data.return_stmt.value);
            break;
        case STMT_PRINT:
            expr_destroy(stmt->data.print_stmt.value);
            break;
        case STMT_BLOCK:
            for (size_t i = 0; i < stmt->data.block.statements.size; i++) {
                stmt_destroy((Stmt*)array_get(&stmt->data.block.statements, i));
            }
            array_free(&stmt->data.block.statements);
            break;
    }
    safe_free(stmt);
}

void parameter_destroy(Parameter* param) {
    if (!param) return;
    safe_free(param->name);
    safe_free(param);
}

void function_destroy(Function* func) {
    if (!func) return;
    safe_free(func->name);
    for (size_t i = 0; i < func->params.size; i++) {
        parameter_destroy((Parameter*)array_get(&func->params, i));
    }
    array_free(&func->params);
    stmt_destroy(func->body);
    safe_free(func);
}

void program_destroy(Program* program) {
    if (!program) return;
    for (size_t i = 0; i < program->functions.size; i++) {
        function_destroy((Function*)array_get(&program->functions, i));
    }
    array_free(&program->functions);
    safe_free(program);
}

static void print_indent(int indent) {
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
}

void expr_print(const Expr* expr, int indent) {
    if (!expr) return;
    
    print_indent(indent);
    printf("Expr(");
    
    switch (expr->type) {
        case EXPR_LITERAL:
            printf("Literal: %lld", expr->data.literal.value.number_value);
            break;
        case EXPR_VARIABLE:
            printf("Variable: %s", expr->data.variable.name);
            break;
        case EXPR_BINARY:
            printf("Binary: %s\n", token_type_to_string(expr->data.binary.operator));
            expr_print(expr->data.binary.left, indent + 1);
            expr_print(expr->data.binary.right, indent + 1);
            break;
        case EXPR_UNARY:
            printf("Unary: %s\n", token_type_to_string(expr->data.unary.operator));
            expr_print(expr->data.unary.operand, indent + 1);
            break;
        case EXPR_CALL:
            printf("Call: %s", expr->data.call.name);
            for (size_t i = 0; i < expr->data.call.args.size; i++) {
                printf("\n");
                expr_print((Expr*)array_get(&expr->data.call.args, i), indent + 1);
            }
            break;
        case EXPR_GROUP:
            printf("Group:\n");
            expr_print(expr->data.group.expression, indent + 1);
            break;
        case EXPR_ARRAY_INDEX:
            printf("ArrayIndex:\n");
            expr_print(expr->data.array_index.array, indent + 1);
            expr_print(expr->data.array_index.index, indent + 1);
            break;
    }
    
    printf(")\n");
}

void stmt_print(const Stmt* stmt, int indent) {
    if (!stmt) return;
    
    print_indent(indent);
    printf("Stmt(");
    
    switch (stmt->type) {
        case STMT_EXPR:
            printf("Expression:\n");
            expr_print(stmt->data.expr.expression, indent + 1);
            break;
        case STMT_VAR_DECL:
            printf("VarDecl: %s: %s\n", stmt->data.var_decl.name, 
                   data_type_to_string(stmt->data.var_decl.type));
            if (stmt->data.var_decl.initializer) {
                expr_print(stmt->data.var_decl.initializer, indent + 1);
            }
            break;
        case STMT_ARRAY_DECL:
            printf("ArrayDecl: %s: %s, size: %d\n", stmt->data.array_decl.name, 
                   data_type_to_string(stmt->data.array_decl.element_type), stmt->data.array_decl.size);
            if (stmt->data.array_decl.initializer) {
                expr_print(stmt->data.array_decl.initializer, indent + 1);
            }
            break;
        case STMT_ASSIGNMENT:
            printf("Assignment: %s\n", stmt->data.assignment.name);
            expr_print(stmt->data.assignment.value, indent + 1);
            break;
        case STMT_ARRAY_ASSIGNMENT:
            printf("ArrayAssignment:\n");
            expr_print(stmt->data.array_assignment.array, indent + 1);
            expr_print(stmt->data.array_assignment.index, indent + 1);
            expr_print(stmt->data.array_assignment.value, indent + 1);
            break;
        case STMT_IF:
            printf("If:\n");
            expr_print(stmt->data.if_stmt.condition, indent + 1);
            stmt_print(stmt->data.if_stmt.then_branch, indent + 1);
            if (stmt->data.if_stmt.else_branch) {
                print_indent(indent);
                printf("Else:\n");
                stmt_print(stmt->data.if_stmt.else_branch, indent + 1);
            }
            break;
        case STMT_WHILE:
            printf("While:\n");
            expr_print(stmt->data.while_stmt.condition, indent + 1);
            stmt_print(stmt->data.while_stmt.body, indent + 1);
            break;
        case STMT_RETURN:
            printf("Return:\n");
            if (stmt->data.return_stmt.value) {
                expr_print(stmt->data.return_stmt.value, indent + 1);
            }
            break;
        case STMT_PRINT:
            printf("Print:\n");
            expr_print(stmt->data.print_stmt.value, indent + 1);
            break;
        case STMT_BLOCK:
            printf("Block:\n");
            for (size_t i = 0; i < stmt->data.block.statements.size; i++) {
                stmt_print((Stmt*)array_get(&stmt->data.block.statements, i), indent + 1);
            }
            break;
    }
    
    printf(")\n");
}

void function_print(const Function* func, int indent) {
    if (!func) return;
    
    print_indent(indent);
    printf("Function: %s -> %s\n", func->name, data_type_to_string(func->return_type));
    
    print_indent(indent + 1);
    printf("Parameters:\n");
    for (size_t i = 0; i < func->params.size; i++) {
        Parameter* param = (Parameter*)array_get(&func->params, i);
        print_indent(indent + 2);
        printf("%s: %s\n", param->name, data_type_to_string(param->type));
    }
    
    print_indent(indent + 1);
    printf("Body:\n");
    stmt_print(func->body, indent + 2);
}

void program_print(const Program* program) {
    if (!program) return;
    
    printf("Program:\n");
    for (size_t i = 0; i < program->functions.size; i++) {
        function_print((Function*)array_get(&program->functions, i), 1);
    }
}

const char* data_type_to_string(DataType type) {
    switch (type) {
        case TYPE_INT: return "int";
        case TYPE_BOOL: return "bool";
        case TYPE_VOID: return "void";
        case TYPE_ARRAY: return "array";
        case TYPE_FLOAT: return "float";
        case TYPE_DOUBLE: return "double";
        default: return "unknown";
    }
}

DataType token_to_data_type(TLTokenType token_type) {
    switch (token_type) {
        case TOKEN_INT: return TYPE_INT;
        case TOKEN_BOOL: return TYPE_BOOL;
        case TOKEN_FLOAT: return TYPE_FLOAT;
        case TOKEN_DOUBLE: return TYPE_DOUBLE;
        default: return TYPE_VOID;
    }
} 