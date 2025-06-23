#include "../include/semantic.h"

SemanticAnalyzer* semantic_create(Program* program, Error* error) {
    SemanticAnalyzer* analyzer = safe_malloc(sizeof(SemanticAnalyzer));
    analyzer->program = program;
    analyzer->error = error;
    analyzer->had_error = false;
    analyzer->current_scope = scope_create(NULL);
    return analyzer;
}

void semantic_destroy(SemanticAnalyzer* analyzer) {
    if (!analyzer) return;
    
    Scope* scope = analyzer->current_scope;
    while (scope) {
        Scope* parent = scope->parent;
        scope_destroy(scope);
        scope = parent;
    }
    
    safe_free(analyzer);
}

bool semantic_analyze(SemanticAnalyzer* analyzer) {
    for (size_t i = 0; i < analyzer->program->functions.size; i++) {
        Function* func = (Function*)array_get(&analyzer->program->functions, i);
        scope_define(analyzer, func->name, SYMBOL_FUNCTION, func->return_type);
    }
    
    for (size_t i = 0; i < analyzer->program->functions.size; i++) {
        Function* func = (Function*)array_get(&analyzer->program->functions, i);
        type_check_function(analyzer, func);
    }
    
    return !analyzer->had_error;
}

Scope* scope_create(Scope* parent) {
    Scope* scope = safe_malloc(sizeof(Scope));
    scope->symbols = hashtable_create(16);
    scope->parent = parent;
    scope->level = parent ? parent->level + 1 : 0;
    return scope;
}

void scope_destroy(Scope* scope) {
    if (!scope) return;
    
    // Note: We don't destroy the symbols here as they're owned by the AST
    hashtable_destroy(scope->symbols);
    safe_free(scope);
}

void scope_enter(SemanticAnalyzer* analyzer) {
    analyzer->current_scope = scope_create(analyzer->current_scope);
}

void scope_exit(SemanticAnalyzer* analyzer) {
    if (!analyzer->current_scope) return;
    
    Scope* parent = analyzer->current_scope->parent;
    scope_destroy(analyzer->current_scope);
    analyzer->current_scope = parent;
}

Symbol* scope_define(SemanticAnalyzer* analyzer, const char* name, SymbolType type, DataType data_type) {
    if (hashtable_contains(analyzer->current_scope->symbols, name)) {
        semantic_error_redefined(analyzer, name, 0, 0); 
        return NULL;
    }
    
    Symbol* symbol = safe_malloc(sizeof(Symbol));
    symbol->name = string_copy(name);
    symbol->type = type;
    symbol->data_type = data_type;
    symbol->scope_level = analyzer->current_scope->level;
    
    hashtable_put(analyzer->current_scope->symbols, name, symbol);
    return symbol;
}

Symbol* scope_resolve(SemanticAnalyzer* analyzer, const char* name) {
    Scope* scope = analyzer->current_scope;
    while (scope) {
        Symbol* symbol = hashtable_get(scope->symbols, name);
        if (symbol) return symbol;
        scope = scope->parent;
    }
    return NULL;
}

DataType type_check_expression(SemanticAnalyzer* analyzer, Expr* expr) {
    if (!expr) return TYPE_VOID;
    
    switch (expr->type) {
        case EXPR_LITERAL:
            return TYPE_INT; 
            
        case EXPR_VARIABLE: {
            Symbol* symbol = scope_resolve(analyzer, expr->data.variable.name);
            if (!symbol) {
                semantic_error_undefined(analyzer, expr->data.variable.name, expr->line, expr->column);
                return TYPE_VOID;
            }
            return symbol->data_type;
        }
        
        case EXPR_BINARY: {
            DataType left_type = type_check_expression(analyzer, expr->data.binary.left);
            DataType right_type = type_check_expression(analyzer, expr->data.binary.right);
            
            if (!type_check_binary(analyzer, expr->data.binary.operator, left_type, right_type)) {
                return TYPE_VOID;
            }
            
            switch (expr->data.binary.operator) {
                case TOKEN_EQ:
                case TOKEN_NE:
                case TOKEN_LT:
                case TOKEN_LE:
                case TOKEN_GT:
                case TOKEN_GE:
                case TOKEN_AND:
                case TOKEN_OR:
                    return TYPE_BOOL;
                default:
                    return TYPE_INT;
            }
        }
        
        case EXPR_UNARY: {
            DataType operand_type = type_check_expression(analyzer, expr->data.unary.operand);
            
            if (!type_check_unary(analyzer, expr->data.unary.operator, operand_type)) {
                return TYPE_VOID;
            }
            
            switch (expr->data.unary.operator) {
                case TOKEN_BANG:
                    return TYPE_BOOL;
                default:
                    return TYPE_INT;
            }
        }
        
        case EXPR_CALL: {
            Symbol* symbol = scope_resolve(analyzer, expr->data.call.name);
            if (!symbol) {
                semantic_error_undefined(analyzer, expr->data.call.name, expr->line, expr->column);
                return TYPE_VOID;
            }
            
            if (symbol->type != SYMBOL_FUNCTION) {
                semantic_error(analyzer, "Can only call functions", expr->line, expr->column);
                return TYPE_VOID;
            }
            
            // Check argument count and types
            // TODO: Implement proper function signature checking - fix this alan
            
            return symbol->data_type;
        }
        
        case EXPR_GROUP:
            return type_check_expression(analyzer, expr->data.group.expression);
            
        default:
            return TYPE_VOID;
    }
}

DataType type_check_statement(SemanticAnalyzer* analyzer, Stmt* stmt) {
    if (!stmt) return TYPE_VOID;
    
    switch (stmt->type) {
        case STMT_EXPR:
            return type_check_expression(analyzer, stmt->data.expr.expression);
            
        case STMT_VAR_DECL: {
            DataType declared_type = stmt->data.var_decl.type;
            
            if (stmt->data.var_decl.initializer) {
                DataType init_type = type_check_expression(analyzer, stmt->data.var_decl.initializer);
                if (!type_check_assignment(analyzer, declared_type, init_type)) {
                    semantic_error_type_mismatch(analyzer, declared_type, init_type, 
                                               stmt->line, stmt->column);
                }
            }
            
            scope_define(analyzer, stmt->data.var_decl.name, SYMBOL_VARIABLE, declared_type);
            return TYPE_VOID;
        }
        
        case STMT_ASSIGNMENT: {
            Symbol* symbol = scope_resolve(analyzer, stmt->data.assignment.name);
            if (!symbol) {
                semantic_error_undefined(analyzer, stmt->data.assignment.name, 
                                       stmt->line, stmt->column);
                return TYPE_VOID;
            }
            
            DataType value_type = type_check_expression(analyzer, stmt->data.assignment.value);
            if (!type_check_assignment(analyzer, symbol->data_type, value_type)) {
                semantic_error_type_mismatch(analyzer, symbol->data_type, value_type, 
                                           stmt->line, stmt->column);
            }
            
            return TYPE_VOID;
        }
        
        case STMT_IF: {
            DataType condition_type = type_check_expression(analyzer, stmt->data.if_stmt.condition);
            if (condition_type != TYPE_BOOL && condition_type != TYPE_INT) {
                semantic_error(analyzer, "If condition must be boolean or integer", stmt->line, stmt->column);
            }
            
            type_check_statement(analyzer, stmt->data.if_stmt.then_branch);
            if (stmt->data.if_stmt.else_branch) {
                type_check_statement(analyzer, stmt->data.if_stmt.else_branch);
            }
            
            return TYPE_VOID;
        }
        
        case STMT_WHILE: {
            DataType condition_type = type_check_expression(analyzer, stmt->data.while_stmt.condition);
            if (condition_type != TYPE_BOOL && condition_type != TYPE_INT) {
                semantic_error(analyzer, "While condition must be boolean or integer", stmt->line, stmt->column);
            }
            
            type_check_statement(analyzer, stmt->data.while_stmt.body);
            return TYPE_VOID;
        }
        
        case STMT_RETURN: {
            if (stmt->data.return_stmt.value) {
                type_check_expression(analyzer, stmt->data.return_stmt.value);
            }
            return TYPE_VOID;
        }
        
        case STMT_BLOCK: {
            scope_enter(analyzer);
            
            for (size_t i = 0; i < stmt->data.block.statements.size; i++) {
                Stmt* block_stmt = (Stmt*)array_get(&stmt->data.block.statements, i);
                type_check_statement(analyzer, block_stmt);
            }
            
            scope_exit(analyzer);
            return TYPE_VOID;
        }
        
        default:
            return TYPE_VOID;
    }
}

DataType type_check_function(SemanticAnalyzer* analyzer, Function* func) {
    scope_enter(analyzer);
    
    for (size_t i = 0; i < func->params.size; i++) {
        Parameter* param = (Parameter*)array_get(&func->params, i);
        scope_define(analyzer, param->name, SYMBOL_PARAMETER, param->type);
    }
    
    type_check_statement(analyzer, func->body);
    
    scope_exit(analyzer);
    return func->return_type;
}

bool type_check_assignment(SemanticAnalyzer* analyzer, DataType target_type, DataType value_type) {
    (void)analyzer; 
    return target_type == value_type;
}

bool type_check_binary(SemanticAnalyzer* analyzer, TokenType operator, DataType left_type, DataType right_type) {
    switch (operator) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_STAR:
        case TOKEN_SLASH:
        case TOKEN_PERCENT:
            if (!is_numeric_type(left_type) || !is_numeric_type(right_type)) {
                semantic_error(analyzer, "Arithmetic operators require numeric operands", 0, 0);
                return false;
            }
            return true;
            
        case TOKEN_EQ:
        case TOKEN_NE:
            if (!types_are_compatible(left_type, right_type)) {
                semantic_error(analyzer, "Cannot compare incompatible types", 0, 0);
                return false;
            }
            return true;
            
        case TOKEN_LT:
        case TOKEN_LE:
        case TOKEN_GT:
        case TOKEN_GE:
            if (!is_numeric_type(left_type) || !is_numeric_type(right_type)) {
                semantic_error(analyzer, "Comparison operators require numeric operands", 0, 0);
                return false;
            }
            return true;
            
        case TOKEN_AND:
        case TOKEN_OR:
            if (!is_boolean_type(left_type) || !is_boolean_type(right_type)) {
                semantic_error(analyzer, "Logical operators require boolean operands", 0, 0);
                return false;
            }
            return true;
            
        default:
            return false;
    }
}

bool type_check_unary(SemanticAnalyzer* analyzer, TokenType operator, DataType operand_type) {
    switch (operator) {
        case TOKEN_MINUS:
            if (!is_numeric_type(operand_type)) {
                semantic_error(analyzer, "Unary minus requires numeric operand", 0, 0);
                return false;
            }
            return true;
            
        case TOKEN_BANG:
            if (!is_boolean_type(operand_type)) {
                semantic_error(analyzer, "Logical not requires boolean operand", 0, 0);
                return false;
            }
            return true;
            
        default:
            return false;
    }
}

void semantic_error(SemanticAnalyzer* analyzer, const char* message, int line, int column) {
    analyzer->had_error = true;
    if (analyzer->error) {
        error_set(analyzer->error, ERROR_SEMANTIC, message, line, column);
    }
}

void semantic_error_type_mismatch(SemanticAnalyzer* analyzer, DataType expected, DataType actual, int line, int column) {
    char message[256];
    snprintf(message, sizeof(message), "Type mismatch: expected %s, got %s", 
             data_type_to_string(expected), data_type_to_string(actual));
    semantic_error(analyzer, message, line, column);
}

void semantic_error_undefined(SemanticAnalyzer* analyzer, const char* name, int line, int column) {
    char message[256];
    snprintf(message, sizeof(message), "Undefined variable '%s'", name);
    semantic_error(analyzer, message, line, column);
}

void semantic_error_redefined(SemanticAnalyzer* analyzer, const char* name, int line, int column) {
    char message[256];
    snprintf(message, sizeof(message), "Variable '%s' already defined", name);
    semantic_error(analyzer, message, line, column);
}

const char* symbol_type_to_string(SymbolType type) {
    switch (type) {
        case SYMBOL_VARIABLE: return "variable";
        case SYMBOL_FUNCTION: return "function";
        case SYMBOL_PARAMETER: return "parameter";
        default: return "unknown";
    }
}

bool is_numeric_type(DataType type) {
    return type == TYPE_INT;
}

bool is_boolean_type(DataType type) {
    return type == TYPE_BOOL;
}

bool types_are_compatible(DataType type1, DataType type2) {
    return type1 == type2;
} 