#include "frontend/aststmt.h"
#include "frontend/astexpr.h"

Stmt *stmt_expr(Expr *expression, int line, int column)
{
    Stmt *stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_EXPR;
    stmt->line = line;
    stmt->column = column;
    stmt->data.expr.expression = expression;
    return stmt;
}

Stmt *stmt_var_decl(const char *name, DataType type, Expr *initializer, int line, int column)
{
    Stmt *stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_VAR_DECL;
    stmt->line = line;
    stmt->column = column;
    stmt->data.var_decl.name = string_copy(name);
    stmt->data.var_decl.type = type;
    stmt->data.var_decl.initializer = initializer;
    return stmt;
}

Stmt *stmt_array_decl(const char *name, DataType element_type, int size, Expr *initializer, int line, int column)
{
    Stmt *stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_ARRAY_DECL;
    stmt->line = line;
    stmt->column = column;
    stmt->data.array_decl.name = string_copy(name);
    stmt->data.array_decl.element_type = element_type;
    stmt->data.array_decl.size = size;
    stmt->data.array_decl.initializer = initializer;
    return stmt;
}

Stmt *stmt_assignment(const char *name, Expr *value, int line, int column)
{
    Stmt *stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_ASSIGNMENT;
    stmt->line = line;
    stmt->column = column;
    stmt->data.assignment.name = string_copy(name);
    stmt->data.assignment.value = value;
    return stmt;
}

Stmt *stmt_array_assignment(Expr *array, Expr *index, Expr *value, int line, int column)
{
    Stmt *stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_ARRAY_ASSIGNMENT;
    stmt->line = line;
    stmt->column = column;
    stmt->data.array_assignment.array = array;
    stmt->data.array_assignment.index = index;
    stmt->data.array_assignment.value = value;
    return stmt;
}

Stmt *stmt_if(Expr *condition, Stmt *then_branch, Stmt *else_branch, int line, int column)
{
    Stmt *stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_IF;
    stmt->line = line;
    stmt->column = column;
    stmt->data.if_stmt.condition = condition;
    stmt->data.if_stmt.then_branch = then_branch;
    stmt->data.if_stmt.else_branch = else_branch;
    return stmt;
}

Stmt *stmt_while(Expr *condition, Stmt *body, int line, int column)
{
    Stmt *stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_WHILE;
    stmt->line = line;
    stmt->column = column;
    stmt->data.while_stmt.condition = condition;
    stmt->data.while_stmt.body = body;
    return stmt;
}

Stmt *stmt_break(int line, int column)
{
    Stmt *stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_BREAK;
    stmt->line = line;
    stmt->column = column;
    return stmt;
}

Stmt *stmt_continue(int line, int column)
{
    Stmt *stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_CONTINUE;
    stmt->line = line;
    stmt->column = column;
    return stmt;
}

Stmt *stmt_return(Expr *value, int line, int column)
{
    Stmt *stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_RETURN;
    stmt->line = line;
    stmt->column = column;
    stmt->data.return_stmt.value = value;
    return stmt;
}

Stmt *stmt_print_stmt(int line, int column)
{
    Stmt *stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_PRINT;
    stmt->line = line;
    stmt->column = column;
    array_init(&stmt->data.print_stmt.args, 4);
    return stmt;
}

Stmt *stmt_include(const char *path, IncludeType type, int line, int column)
{
    Stmt *stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_INCLUDE;
    stmt->line = line;
    stmt->column = column;
    stmt->data.include.path = string_copy(path);
    stmt->data.include.type = type;
    return stmt;
}

Stmt *stmt_block(int line, int column)
{
    Stmt *stmt = safe_malloc(sizeof(Stmt));
    stmt->type = STMT_BLOCK;
    stmt->line = line;
    stmt->column = column;
    array_init(&stmt->data.block.statements, 8);
    return stmt;
}

void stmt_add_block_stmt(Stmt *block, Stmt *stmt)
{
    array_push(&block->data.block.statements, stmt);
}

void stmt_add_print_arg(Stmt *print_stmt, Expr *arg)
{
    array_push(&print_stmt->data.print_stmt.args, arg);
}

void stmt_destroy(Stmt *stmt)
{
    if (!stmt)
        return;

    switch (stmt->type)
    {
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
    case STMT_BREAK:
    case STMT_CONTINUE:
        break;
    case STMT_RETURN:
        expr_destroy(stmt->data.return_stmt.value);
        break;
    case STMT_PRINT:
        for (size_t i = 0; i < stmt->data.print_stmt.args.size; i++)
        {
            expr_destroy((Expr *)array_get(&stmt->data.print_stmt.args, i));
        }
        array_free(&stmt->data.print_stmt.args);
        break;
    case STMT_BLOCK:
        for (size_t i = 0; i < stmt->data.block.statements.size; i++)
        {
            stmt_destroy((Stmt *)array_get(&stmt->data.block.statements, i));
        }
        array_free(&stmt->data.block.statements);
        break;
    case STMT_INCLUDE:
        safe_free(stmt->data.include.path);
        break;
    }
    safe_free(stmt);
}

Stmt *stmt_copy(Stmt *stmt)
{
    if (!stmt)
        return NULL;

    Stmt *copy = safe_malloc(sizeof(Stmt));
    copy->type = stmt->type;
    copy->line = stmt->line;
    copy->column = stmt->column;

    switch (stmt->type)
    {
    case STMT_EXPR:
        copy->data.expr.expression = expr_copy(stmt->data.expr.expression);
        break;
    case STMT_VAR_DECL:
        copy->data.var_decl.name = string_copy(stmt->data.var_decl.name);
        copy->data.var_decl.type = stmt->data.var_decl.type;
        copy->data.var_decl.initializer = expr_copy(stmt->data.var_decl.initializer);
        break;
    case STMT_ARRAY_DECL:
        copy->data.array_decl.name = string_copy(stmt->data.array_decl.name);
        copy->data.array_decl.element_type = stmt->data.array_decl.element_type;
        copy->data.array_decl.size = stmt->data.array_decl.size;
        copy->data.array_decl.initializer = expr_copy(stmt->data.array_decl.initializer);
        break;
    case STMT_ASSIGNMENT:
        copy->data.assignment.name = string_copy(stmt->data.assignment.name);
        copy->data.assignment.value = expr_copy(stmt->data.assignment.value);
        break;
    case STMT_ARRAY_ASSIGNMENT:
        copy->data.array_assignment.array = expr_copy(stmt->data.array_assignment.array);
        copy->data.array_assignment.index = expr_copy(stmt->data.array_assignment.index);
        copy->data.array_assignment.value = expr_copy(stmt->data.array_assignment.value);
        break;
    case STMT_IF:
        copy->data.if_stmt.condition = expr_copy(stmt->data.if_stmt.condition);
        copy->data.if_stmt.then_branch = stmt_copy(stmt->data.if_stmt.then_branch);
        copy->data.if_stmt.else_branch = stmt_copy(stmt->data.if_stmt.else_branch);
        break;
    case STMT_WHILE:
        copy->data.while_stmt.condition = expr_copy(stmt->data.while_stmt.condition);
        copy->data.while_stmt.body = stmt_copy(stmt->data.while_stmt.body);
        break;
    case STMT_RETURN:
        copy->data.return_stmt.value = expr_copy(stmt->data.return_stmt.value);
        break;
    case STMT_PRINT:
        array_init(&copy->data.print_stmt.args, stmt->data.print_stmt.args.size);
        for (size_t i = 0; i < stmt->data.print_stmt.args.size; i++)
        {
            Expr *arg = (Expr *)array_get(&stmt->data.print_stmt.args, i);
            array_push(&copy->data.print_stmt.args, expr_copy(arg));
        }
        break;
    case STMT_BLOCK:
        array_init(&copy->data.block.statements, stmt->data.block.statements.size);
        for (size_t i = 0; i < stmt->data.block.statements.size; i++)
        {
            Stmt *block_stmt = (Stmt *)array_get(&stmt->data.block.statements, i);
            array_push(&copy->data.block.statements, stmt_copy(block_stmt));
        }
        break;
    case STMT_BREAK:
    case STMT_CONTINUE:
        // No additional data to copy
        break;
    case STMT_INCLUDE:
        copy->data.include.path = string_copy(stmt->data.include.path);
        copy->data.include.type = stmt->data.include.type;
        break;
    }

    return copy;
}

static void print_indent(int indent)
{
    for (int i = 0; i < indent; i++)
    {
        printf("  ");
    }
}

void stmt_print(const Stmt *stmt, int indent)
{
    if (!stmt)
        return;

    print_indent(indent);
    printf("Stmt(");

    switch (stmt->type)
    {
    case STMT_EXPR:
        printf("Expression:\n");
        expr_print(stmt->data.expr.expression, indent + 1);
        break;
    case STMT_VAR_DECL:
        printf("VarDecl: %s: %s\n", stmt->data.var_decl.name,
               data_type_to_string(stmt->data.var_decl.type));
        if (stmt->data.var_decl.initializer)
        {
            expr_print(stmt->data.var_decl.initializer, indent + 1);
        }
        break;
    case STMT_ARRAY_DECL:
        printf("ArrayDecl: %s: %s, size: %d\n", stmt->data.array_decl.name,
               data_type_to_string(stmt->data.array_decl.element_type), stmt->data.array_decl.size);
        if (stmt->data.array_decl.initializer)
        {
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
        if (stmt->data.if_stmt.else_branch)
        {
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
    case STMT_BREAK:
        printf("Break");
        break;
    case STMT_CONTINUE:
        printf("Continue");
        break;
    case STMT_RETURN:
        printf("Return:\n");
        if (stmt->data.return_stmt.value)
        {
            expr_print(stmt->data.return_stmt.value, indent + 1);
        }
        break;
    case STMT_PRINT:
        printf("Print:\n");
        for (size_t i = 0; i < stmt->data.print_stmt.args.size; i++)
        {
            expr_print((Expr *)array_get(&stmt->data.print_stmt.args, i), indent + 1);
        }
        break;
    case STMT_BLOCK:
        printf("Block:\n");
        for (size_t i = 0; i < stmt->data.block.statements.size; i++)
        {
            stmt_print((Stmt *)array_get(&stmt->data.block.statements, i), indent + 1);
        }
        break;
    case STMT_INCLUDE:
        printf("Include: %s (%s)", stmt->data.include.path,
               stmt->data.include.type == INCLUDE_SYSTEM ? "system" : "local");
        break;
    }

    printf(")\n");
}