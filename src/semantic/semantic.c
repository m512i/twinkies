#include "../include/semantic.h"
#include <stdarg.h>

static Symbol* scope_define_function_overload(SemanticAnalyzer* analyzer, Function* func);
static Symbol* resolve_function_overload(SemanticAnalyzer* analyzer, const char* name, DynamicArray* arg_types);
static void make_signature_string(DynamicArray* params, char* buf, size_t buflen);
static bool parameter_list_equals(DynamicArray* a, DynamicArray* b);
static DynamicArray* get_or_create_overload_set(SemanticAnalyzer* analyzer, const char* name);

SemanticAnalyzer* semantic_create(Program* program, ErrorContext* error_context) {
    SemanticAnalyzer* analyzer = safe_malloc(sizeof(SemanticAnalyzer));
    analyzer->program = program;
    analyzer->error_context = error_context;
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

static void check_unused_variables(SemanticAnalyzer* analyzer, Scope* scope) {
    for (size_t i = 0; i < scope->symbols->capacity; i++) {
        HashTableEntry* entry = scope->symbols->buckets[i];
        while (entry) {
            Symbol* symbol = (Symbol*)entry->value;
            if (symbol && symbol->type == SYMBOL_VARIABLE && symbol->is_defined && !symbol->is_used) {
                semantic_warning_unused_variable(analyzer, symbol->name, 
                                               symbol->definition_line, symbol->definition_column);
            }
            entry = entry->next;
        }
    }
}

bool semantic_analyze(SemanticAnalyzer* analyzer) {
    for (size_t i = 0; i < analyzer->program->functions.size; i++) {
        Function* func = (Function*)array_get(&analyzer->program->functions, i);
        scope_define_function_overload(analyzer, func);
    }
    for (size_t i = 0; i < analyzer->program->functions.size; i++) {
        Function* func = (Function*)array_get(&analyzer->program->functions, i);
        type_check_function(analyzer, func);
    }
    
    Scope* scope = analyzer->current_scope;
    while (scope) {
        check_unused_variables(analyzer, scope);
        scope = scope->parent;
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
    symbol->array_size = -1;  
    symbol->is_used = false;
    symbol->is_defined = true;
    symbol->definition_line = 0; 
    symbol->definition_column = 0; 
    
    hashtable_put(analyzer->current_scope->symbols, name, symbol);
    return symbol;
}

Symbol* scope_define_array(SemanticAnalyzer* analyzer, const char* name, DataType element_type, int size) {
    printf("[DEBUG] Defining array %s with size %d\n", name, size);
    if (hashtable_contains(analyzer->current_scope->symbols, name)) {
        semantic_error_redefined(analyzer, name, 0, 0); 
        return NULL;
    }
    
    Symbol* symbol = safe_malloc(sizeof(Symbol));
    symbol->name = string_copy(name);
    symbol->type = SYMBOL_VARIABLE;
    symbol->data_type = TYPE_ARRAY;
    symbol->scope_level = analyzer->current_scope->level;
    symbol->array_size = size;  
    symbol->is_used = false;
    symbol->is_defined = true;
    symbol->definition_line = 0;  
    symbol->definition_column = 0;
    
    hashtable_put(analyzer->current_scope->symbols, name, symbol);
    printf("[DEBUG] Array %s defined with size %d\n", name, symbol->array_size);
    
    // Debug: check if the symbol is actually in the hashtable
    Symbol* check = hashtable_get(analyzer->current_scope->symbols, name);
    if (check) {
        printf("[DEBUG] Verified: symbol %s is in hashtable with size %d\n", name, check->array_size);
    } else {
        printf("[DEBUG] ERROR: symbol %s is NOT in hashtable after putting it!\n", name);
    }
    
    return symbol;
}

Symbol* scope_resolve(SemanticAnalyzer* analyzer, const char* name) {
    printf("[DEBUG] Resolving symbol %s\n", name);
    Scope* scope = analyzer->current_scope;
    int scope_level = 0;
    while (scope) {
        printf("[DEBUG] Checking scope level %d\n", scope_level);
        printf("[DEBUG] Scope hashtable size: %zu\n", scope->symbols->size);
        
        // Debug: print all keys in this scope
        printf("[DEBUG] Keys in scope level %d:\n", scope_level);
        for (size_t i = 0; i < scope->symbols->capacity; i++) {
            HashTableEntry* entry = scope->symbols->buckets[i];
            while (entry) {
                printf("[DEBUG]   Key: '%s', Value: %p\n", entry->key, entry->value);
                entry = entry->next;
            }
        }
        
        Symbol* symbol = hashtable_get(scope->symbols, name);
        if (symbol) {
            printf("[DEBUG] Found symbol %s in scope level %d\n", name, scope_level);
            return symbol;
        }
        scope = scope->parent;
        scope_level++;
    }
    printf("[DEBUG] Symbol %s not found in any scope\n", name);
    return NULL;
}

int get_array_size(SemanticAnalyzer* analyzer, const char* name) {
    printf("[DEBUG] Looking up array size for %s\n", name);
    Symbol* symbol = scope_resolve(analyzer, name);
    if (symbol) {
        printf("[DEBUG] Found symbol %s, type=%d, array_size=%d\n", name, symbol->data_type, symbol->array_size);
    } else {
        printf("[DEBUG] Symbol %s not found\n", name);
    }
    if (symbol && symbol->data_type == TYPE_ARRAY) {
        printf("[DEBUG] Returning array size %d for %s\n", symbol->array_size, name);
        return symbol->array_size;
    }
    printf("[DEBUG] Returning -1 for %s (not an array or not found)\n", name);
    return -1; 
}

DataType type_check_expression(SemanticAnalyzer* analyzer, Expr* expr) {
    if (!expr) return TYPE_VOID;
    
    switch (expr->type) {
        case EXPR_LITERAL:
            if (expr->data.literal.is_bool_literal) {
                return TYPE_BOOL;
            } else {
                return TYPE_INT;
            }
            
        case EXPR_VARIABLE: {
            Symbol* symbol = scope_resolve(analyzer, expr->data.variable.name);
            if (!symbol) {
                semantic_error_undefined(analyzer, expr->data.variable.name, expr->line, expr->column);
                return TYPE_VOID;
            }
            
            symbol->is_used = true;
            
            return symbol->data_type;
        }
        
        case EXPR_BINARY: {
            DataType left_type = type_check_expression(analyzer, expr->data.binary.left);
            DataType right_type = type_check_expression(analyzer, expr->data.binary.right);
            
            if (left_type == TYPE_VOID || right_type == TYPE_VOID) {
                return TYPE_VOID; 
            }
            
            if (expr->data.binary.operator == TOKEN_SLASH || expr->data.binary.operator == TOKEN_PERCENT) {
                if (expr->data.binary.right->type == EXPR_LITERAL) {
                    int64_t value = expr->data.binary.right->data.literal.value.number_value;
                    if (value == 0) {
                        semantic_warning_performance(analyzer, "Division by zero detected", expr->line, expr->column);
                    } else if (value == 1 && expr->data.binary.operator == TOKEN_SLASH) {
                        semantic_warning_performance(analyzer, "Division by 1 is unnecessary", expr->line, expr->column);
                    }
                }
            }
            
            if (!type_check_binary(analyzer, expr->data.binary.operator, left_type, right_type, expr->line, expr->column)) {
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
            
            if (operand_type == TYPE_VOID) {
                return TYPE_VOID; 
            }
            
            if (!type_check_unary(analyzer, expr->data.unary.operator, operand_type, expr->line, expr->column)) {
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
            DynamicArray arg_types;
            array_init(&arg_types, expr->data.call.args.size);
            bool args_valid = true;
            
            for (size_t i = 0; i < expr->data.call.args.size; i++) {
                Expr* arg_expr = (Expr*)array_get(&expr->data.call.args, i);
                DataType arg_type = type_check_expression(analyzer, arg_expr);
                if (arg_type == TYPE_VOID) {
                    args_valid = false;
                }
                Parameter* p = safe_malloc(sizeof(Parameter));
                p->name = NULL;
                p->type = arg_type;
                array_push(&arg_types, p);
            }
            
            if (!args_valid) {
                for (size_t i = 0; i < arg_types.size; i++) safe_free(array_get(&arg_types, i));
                array_free(&arg_types);
                return TYPE_VOID;
            }
            
            Symbol* symbol = resolve_function_overload(analyzer, expr->data.call.name, &arg_types);
            for (size_t i = 0; i < arg_types.size; i++) safe_free(array_get(&arg_types, i));
            array_free(&arg_types);
            
            if (!symbol) {
                char sig[128];
                sig[0] = '\0';
                for (size_t i = 0; i < expr->data.call.args.size; i++) {
                    Expr* arg_expr = (Expr*)array_get(&expr->data.call.args, i);
                    DataType arg_type = type_check_expression(analyzer, arg_expr);
                    strncat(sig, data_type_to_string(arg_type), sizeof(sig) - strlen(sig) - 1);
                    if (i + 1 < expr->data.call.args.size) strncat(sig, ",", sizeof(sig) - strlen(sig) - 1);
                }
                char msg[256];
                snprintf(msg, sizeof(msg), "No matching overload for function '%s' with argument types: (%s)", expr->data.call.name, sig);
                semantic_error(analyzer, msg, expr->line, expr->column);
                return TYPE_VOID;
            }
            return symbol->data_type;
        }
        
        case EXPR_GROUP:
            return type_check_expression(analyzer, expr->data.group.expression);
            
        case EXPR_ARRAY_INDEX: {
            DataType array_type = type_check_expression(analyzer, expr->data.array_index.array);
            DataType index_type = type_check_expression(analyzer, expr->data.array_index.index);
            
            if (array_type == TYPE_VOID || index_type == TYPE_VOID) {
                return TYPE_VOID; 
            }
            
            if (array_type != TYPE_ARRAY) {
                semantic_error_invalid_operation(analyzer, "[]", array_type, expr->line, expr->column);
                return TYPE_VOID;
            }
            
            if (index_type != TYPE_INT) {
                semantic_error(analyzer, "Array index must be integer", expr->line, expr->column);
                return TYPE_VOID;
            }
            
            // For now, return TYPE_INT as the element type
            // In a full implementation, you'd track the actual element type
            return TYPE_INT;
        }
            
        default:
            return TYPE_VOID;
    }
}

DataType type_check_statement(SemanticAnalyzer* analyzer, Stmt* stmt) {
    if (!stmt) return TYPE_VOID;
    
    static bool has_return = false;  
    
    switch (stmt->type) {
        case STMT_EXPR:
            return type_check_expression(analyzer, stmt->data.expr.expression);
            
        case STMT_VAR_DECL: {
            if (has_return) {
                semantic_warning_unreachable_code(analyzer, stmt->line, stmt->column);
            }
            
            DataType declared_type = stmt->data.var_decl.type;
            if (stmt->data.var_decl.initializer) {
                DataType init_type = type_check_expression(analyzer, stmt->data.var_decl.initializer);
                if (init_type != TYPE_VOID && !type_check_assignment(analyzer, declared_type, init_type)) {
                    semantic_error_type_mismatch(analyzer, declared_type, init_type, 
                                               stmt->line, stmt->column);
                } else if (init_type != TYPE_VOID && declared_type != init_type) {
                    if (declared_type == TYPE_INT && init_type == TYPE_BOOL) {
                        semantic_warning_type_conversion(analyzer, init_type, declared_type, stmt->line, stmt->column);
                    } else if (declared_type == TYPE_BOOL && init_type == TYPE_INT) {
                        semantic_warning_type_conversion(analyzer, init_type, declared_type, stmt->line, stmt->column);
                    }
                }
            }
            Symbol* symbol = scope_define(analyzer, stmt->data.var_decl.name, SYMBOL_VARIABLE, declared_type);
            if (symbol) {
                symbol->definition_line = stmt->line;
                symbol->definition_column = stmt->column;
            }
            return TYPE_VOID;
        }
        
        case STMT_ARRAY_DECL: {
            if (has_return) {
                semantic_warning_unreachable_code(analyzer, stmt->line, stmt->column);
            }
            
            if (stmt->data.array_decl.initializer) {
                DataType init_type = type_check_expression(analyzer, stmt->data.array_decl.initializer);
                if (init_type != TYPE_VOID && init_type != TYPE_ARRAY && init_type != stmt->data.array_decl.element_type) {
                    semantic_error_type_mismatch(analyzer, stmt->data.array_decl.element_type, init_type, 
                                               stmt->line, stmt->column);
                }
            }
            Symbol* symbol = scope_define_array(analyzer, stmt->data.array_decl.name, stmt->data.array_decl.element_type, stmt->data.array_decl.size);
            if (symbol) {
                symbol->definition_line = stmt->line;
                symbol->definition_column = stmt->column;
            }
            return TYPE_VOID;
        }
        
        case STMT_ASSIGNMENT: {
            if (has_return) {
                semantic_warning_unreachable_code(analyzer, stmt->line, stmt->column);
            }
            
            Symbol* symbol = scope_resolve(analyzer, stmt->data.assignment.name);
            if (!symbol) {
                semantic_error_undefined(analyzer, stmt->data.assignment.name, 
                                       stmt->line, stmt->column);
                return TYPE_VOID;
            }
            DataType value_type = type_check_expression(analyzer, stmt->data.assignment.value);
            if (value_type != TYPE_VOID && !type_check_assignment(analyzer, symbol->data_type, value_type)) {
                semantic_error_type_mismatch(analyzer, symbol->data_type, value_type, 
                                           stmt->line, stmt->column);
            }
            return TYPE_VOID;
        }
        
        case STMT_ARRAY_ASSIGNMENT: {
            if (has_return) {
                semantic_warning_unreachable_code(analyzer, stmt->line, stmt->column);
            }
            
            DataType array_type = type_check_expression(analyzer, stmt->data.array_assignment.array);
            DataType index_type = type_check_expression(analyzer, stmt->data.array_assignment.index);
            DataType value_type = type_check_expression(analyzer, stmt->data.array_assignment.value);
            
            if (array_type == TYPE_VOID || index_type == TYPE_VOID || value_type == TYPE_VOID) {
                return TYPE_VOID; 
            }
            
            if (array_type != TYPE_ARRAY) {
                semantic_error_invalid_operation(analyzer, "[]", array_type, stmt->line, stmt->column);
                return TYPE_VOID;
            }
            
            if (index_type != TYPE_INT) {
                semantic_error(analyzer, "Array index must be integer", stmt->line, stmt->column);
                return TYPE_VOID;
            }
            
            if (!type_check_assignment(analyzer, TYPE_INT, value_type)) {
                semantic_error_type_mismatch(analyzer, TYPE_INT, value_type, 
                                           stmt->line, stmt->column);
            }
            return TYPE_VOID;
        }
        
        case STMT_IF: {
            if (has_return) {
                semantic_warning_unreachable_code(analyzer, stmt->line, stmt->column);
            }
            
            DataType condition_type = type_check_expression(analyzer, stmt->data.if_stmt.condition);
            if (condition_type != TYPE_VOID && condition_type != TYPE_BOOL && condition_type != TYPE_INT) {
                semantic_error(analyzer, "If condition must be boolean or integer", stmt->line, stmt->column);
            }
            
            type_check_statement(analyzer, stmt->data.if_stmt.then_branch);
            if (stmt->data.if_stmt.else_branch) {
                type_check_statement(analyzer, stmt->data.if_stmt.else_branch);
            }
            
            return TYPE_VOID;
        }
        
        case STMT_WHILE: {
            if (has_return) {
                semantic_warning_unreachable_code(analyzer, stmt->line, stmt->column);
            }
            
            DataType condition_type = type_check_expression(analyzer, stmt->data.while_stmt.condition);
            if (condition_type != TYPE_VOID && condition_type != TYPE_BOOL && condition_type != TYPE_INT) {
                semantic_error(analyzer, "While condition must be boolean or integer", stmt->line, stmt->column);
            }
            
            type_check_statement(analyzer, stmt->data.while_stmt.body);
            return TYPE_VOID;
        }
        
        case STMT_RETURN: {
            has_return = true;  
            
            if (stmt->data.return_stmt.value) {
                type_check_expression(analyzer, stmt->data.return_stmt.value);
            }
            return TYPE_VOID;
        }
        
        case STMT_PRINT: {
            if (has_return) {
                semantic_warning_unreachable_code(analyzer, stmt->line, stmt->column);
            }
            
            if (stmt->data.print_stmt.value) {
                type_check_expression(analyzer, stmt->data.print_stmt.value);
            }
            return TYPE_VOID;
        }
        
        case STMT_BLOCK: {
            bool created_scope = false;
            if (analyzer->current_scope->level == 0) {
                scope_enter(analyzer);
                created_scope = true;
            }
            
            for (size_t i = 0; i < stmt->data.block.statements.size; i++) {
                Stmt* block_stmt = (Stmt*)array_get(&stmt->data.block.statements, i);
                type_check_statement(analyzer, block_stmt);
            }
            
            if (created_scope) {
                scope_exit(analyzer);
            }
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
    
    // Don't destroy the scope here - keep it for IR generation
    // scope_exit(analyzer);
    return func->return_type;
}

bool type_check_assignment(SemanticAnalyzer* analyzer, DataType target_type, DataType value_type) {
    if (target_type == value_type) {
        return true;
    }
    
    if (target_type == TYPE_INT && value_type == TYPE_BOOL) {
        // Note: We don't have line/column info here, so we'll use 0,0
        // The actual line info should come from the calling context
        semantic_warning_type_conversion(analyzer, value_type, target_type, 0, 0);
        return true;  // Allow implicit bool to int conversion
    } else if (target_type == TYPE_BOOL && value_type == TYPE_INT) {
        // Note: We don't have line/column info here, so we'll use 0,0
        // The actual line info should come from the calling context
        semantic_warning_type_conversion(analyzer, value_type, target_type, 0, 0);
        return true;  
    }
    
    return false;
}

bool type_check_binary(SemanticAnalyzer* analyzer, TokenType operator, DataType left_type, DataType right_type, int line, int column) {
    switch (operator) {
        case TOKEN_PLUS:
        case TOKEN_MINUS:
        case TOKEN_STAR:
        case TOKEN_SLASH:
        case TOKEN_PERCENT:
            if (!is_numeric_type(left_type) || !is_numeric_type(right_type)) {
                semantic_error(analyzer, "Arithmetic operators require numeric operands", line, column);
                return false;
            }
            return true;
            
        case TOKEN_EQ:
        case TOKEN_NE:
            if (!types_are_compatible(left_type, right_type)) {
                semantic_error(analyzer, "Cannot compare incompatible types", line, column);
                return false;
            }
            return true;
            
        case TOKEN_LT:
        case TOKEN_LE:
        case TOKEN_GT:
        case TOKEN_GE:
            if (!is_numeric_type(left_type) || !is_numeric_type(right_type)) {
                semantic_error(analyzer, "Comparison operators require numeric operands", line, column);
                return false;
            }
            return true;
            
        case TOKEN_AND:
        case TOKEN_OR:
            if (!is_boolean_type(left_type) || !is_boolean_type(right_type)) {
                semantic_error(analyzer, "Logical operators require boolean operands", line, column);
                return false;
            }
            return true;
            
        default:
            return false;
    }
}

bool type_check_unary(SemanticAnalyzer* analyzer, TokenType operator, DataType operand_type, int line, int column) {
    switch (operator) {
        case TOKEN_MINUS:
            if (!is_numeric_type(operand_type)) {
                semantic_error(analyzer, "Unary minus requires numeric operand", line, column);
                return false;
            }
            return true;
            
        case TOKEN_BANG:
            if (!is_boolean_type(operand_type)) {
                semantic_error(analyzer, "Logical not requires boolean operand", line, column);
                return false;
            }
            return true;
            
        default:
            return false;
    }
}

void semantic_error(SemanticAnalyzer* analyzer, const char* message, int line, int column) {
    analyzer->had_error = true;
    if (analyzer->error_context) {
        error_context_add_error(analyzer->error_context, ERROR_SEMANTIC, SEVERITY_ERROR, message, NULL, line, column);
    }
}

void semantic_error_with_suggestion(SemanticAnalyzer* analyzer, const char* message, const char* suggestion, int line, int column) {
    analyzer->had_error = true;
    if (analyzer->error_context) {
        error_context_add_error(analyzer->error_context, ERROR_SEMANTIC, SEVERITY_ERROR, message, suggestion, line, column);
    }
}

void semantic_error_type_mismatch(SemanticAnalyzer* analyzer, DataType expected, DataType actual, int line, int column) {
    char message[256];
    char suggestion[256];
    
    snprintf(message, sizeof(message), "Type mismatch: expected %s, got %s", 
             data_type_to_string(expected), data_type_to_string(actual));
    
    if (expected == TYPE_INT && actual == TYPE_BOOL) {
        snprintf(suggestion, sizeof(suggestion), "Use explicit conversion or comparison operators (==, !=, <, >, etc.)");
    } else if (expected == TYPE_BOOL && actual == TYPE_INT) {
        snprintf(suggestion, sizeof(suggestion), "Use comparison operators (==, !=, <, >, etc.) to convert to boolean");
    } else if (expected == TYPE_ARRAY && actual != TYPE_ARRAY) {
        snprintf(suggestion, sizeof(suggestion), "Use array initialization syntax: 'let arr: int[size] = value' or 'let arr: int[size] = {val1, val2, ...}'");
    } else {
        snprintf(suggestion, sizeof(suggestion), "Check variable declarations and ensure types match");
    }
    
    semantic_error_with_suggestion(analyzer, message, suggestion, line, column);
}

void semantic_error_undefined(SemanticAnalyzer* analyzer, const char* name, int line, int column) {
    char message[256];
    char suggestion[256];
    
    snprintf(message, sizeof(message), "Undefined variable '%s'", name);
    
    Scope* scope = analyzer->current_scope;
    while (scope) {
        for (size_t i = 0; i < scope->symbols->capacity; i++) {
            HashTableEntry* entry = scope->symbols->buckets[i];
            while (entry) {
                Symbol* symbol = (Symbol*)entry->value;
                if (symbol && symbol->name) {
                    if (strlen(symbol->name) == strlen(name) + 1 || 
                        strlen(symbol->name) == strlen(name) - 1 ||
                        strlen(symbol->name) == strlen(name)) {
                        int diff = 0;
                        const char* s1 = symbol->name;
                        const char* s2 = name;
                        while (*s1 && *s2) {
                            if (*s1 != *s2) diff++;
                            s1++;
                            s2++;
                        }
                        if (diff <= 1) {
                            snprintf(suggestion, sizeof(suggestion), "Did you mean '%s'?", symbol->name);
                            semantic_error_with_suggestion(analyzer, message, suggestion, line, column);
                            return;
                        }
                    }
                }
                entry = entry->next;
            }
        }
        scope = scope->parent;
    }
    
    snprintf(suggestion, sizeof(suggestion), "Declare the variable with 'let %s: type;' before using it", name);
    semantic_error_with_suggestion(analyzer, message, suggestion, line, column);
}

void semantic_error_redefined(SemanticAnalyzer* analyzer, const char* name, int line, int column) {
    char message[256];
    char suggestion[256];
    
    snprintf(message, sizeof(message), "Variable '%s' already defined", name);
    snprintf(suggestion, sizeof(suggestion), "Use a different variable name or remove the duplicate declaration");
    
    semantic_error_with_suggestion(analyzer, message, suggestion, line, column);
}

void semantic_error_array_bounds(SemanticAnalyzer* analyzer, const char* array_name, int index, int size, int line, int column) {
    char message[256];
    char suggestion[256];
    
    snprintf(message, sizeof(message), "Array index %d out of bounds for array '%s' (size: %d)", 
             index, array_name, size);
    
    if (index < 0) {
        snprintf(suggestion, sizeof(suggestion), "Array indices must be non-negative. Use a positive index.");
    } else if (index >= size) {
        snprintf(suggestion, sizeof(suggestion), "Valid indices for this array are 0 to %d", size - 1);
    } else {
        snprintf(suggestion, sizeof(suggestion), "Check your array bounds and ensure indices are within range");
    }
    
    semantic_error_with_suggestion(analyzer, message, suggestion, line, column);
}

void semantic_error_invalid_operation(SemanticAnalyzer* analyzer, const char* operation, DataType type, int line, int column) {
    char message[256];
    char suggestion[256];
    
    snprintf(message, sizeof(message), "Invalid operation '%s' on type %s", operation, data_type_to_string(type));
    
    if (strcmp(operation, "+") == 0 || strcmp(operation, "-") == 0 || 
        strcmp(operation, "*") == 0 || strcmp(operation, "/") == 0) {
        snprintf(suggestion, sizeof(suggestion), "Arithmetic operations are only valid on numeric types (int)");
    } else if (strcmp(operation, "&&") == 0 || strcmp(operation, "||") == 0) {
        snprintf(suggestion, sizeof(suggestion), "Logical operations are only valid on boolean types");
    } else if (strcmp(operation, "[]") == 0) {
        snprintf(suggestion, sizeof(suggestion), "Array indexing is only valid on array types");
    } else {
        snprintf(suggestion, sizeof(suggestion), "Check the operation and ensure it's compatible with the data type");
    }
    
    semantic_error_with_suggestion(analyzer, message, suggestion, line, column);
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

static bool parameter_list_equals(DynamicArray* a, DynamicArray* b) {
    if (a->size != b->size) return false;
    for (size_t i = 0; i < a->size; i++) {
        Parameter* pa = (Parameter*)array_get(a, i);
        Parameter* pb = (Parameter*)array_get(b, i);
        if (pa->type != pb->type) return false;
    }
    return true;
}

// Helper: Make a signature string for a function (for error messages)
static void make_signature_string(DynamicArray* params, char* buf, size_t buflen) {
    buf[0] = '\0';
    for (size_t i = 0; i < params->size; i++) {
        Parameter* p = (Parameter*)array_get(params, i);
        strncat(buf, data_type_to_string(p->type), buflen - strlen(buf) - 1);
        if (i + 1 < params->size) strncat(buf, ",", buflen - strlen(buf) - 1);
    }
}

static DynamicArray* get_or_create_overload_set(SemanticAnalyzer* analyzer, const char* name) {
    DynamicArray* overloads = hashtable_get(analyzer->current_scope->symbols, name);
    if (!overloads) {
        overloads = safe_malloc(sizeof(DynamicArray));
        array_init(overloads, 2);
        hashtable_put(analyzer->current_scope->symbols, name, overloads);
    }
    return overloads;
}

static Symbol* scope_define_function_overload(SemanticAnalyzer* analyzer, Function* func) {
    DynamicArray* overloads = get_or_create_overload_set(analyzer, func->name);
    for (size_t i = 0; i < overloads->size; i++) {
        Symbol* sym = (Symbol*)array_get(overloads, i);
        if (parameter_list_equals(&sym->data.function.params, &func->params)) {
            char sig[128];
            make_signature_string(&func->params, sig, sizeof(sig));
            char msg[256];
            snprintf(msg, sizeof(msg), "Function '%s(%s)' already defined", func->name, sig);
            semantic_error_redefined(analyzer, msg, 0, 0);
            return NULL;
        }
    }
    Symbol* sym = safe_malloc(sizeof(Symbol));
    sym->name = string_copy(func->name);
    sym->type = SYMBOL_FUNCTION;
    sym->data_type = func->return_type;
    sym->scope_level = analyzer->current_scope->level;
    array_init(&sym->data.function.params, func->params.size);
    for (size_t j = 0; j < func->params.size; j++) {
        Parameter* param = (Parameter*)array_get(&func->params, j);
        Parameter* param_copy = safe_malloc(sizeof(Parameter));
        param_copy->name = string_copy(param->name);
        param_copy->type = param->type;
        array_push(&sym->data.function.params, param_copy);
    }
    array_push(overloads, sym);
    return sym;
}

static Symbol* resolve_function_overload(SemanticAnalyzer* analyzer, const char* name, DynamicArray* arg_types) {
    Scope* scope = analyzer->current_scope;
    while (scope) {
        DynamicArray* overloads = hashtable_get(scope->symbols, name);
        if (overloads) {
            for (size_t i = 0; i < overloads->size; i++) {
                Symbol* sym = (Symbol*)array_get(overloads, i);
                if (parameter_list_equals(&sym->data.function.params, arg_types)) {
                    return sym;
                }
            }
        }
        scope = scope->parent;
    }
    return NULL;
}

void semantic_warning(SemanticAnalyzer* analyzer, const char* message, int line, int column) {
    if (analyzer->error_context) {
        error_context_add_error(analyzer->error_context, ERROR_SEMANTIC, SEVERITY_WARNING, message, NULL, line, column);
    }
}

void semantic_warning_with_suggestion(SemanticAnalyzer* analyzer, const char* message, const char* suggestion, int line, int column) {
    if (analyzer->error_context) {
        error_context_add_error(analyzer->error_context, ERROR_SEMANTIC, SEVERITY_WARNING, message, suggestion, line, column);
    }
}

void semantic_warning_unused_variable(SemanticAnalyzer* analyzer, const char* name, int line, int column) {
    char message[256];
    char suggestion[256];
    
    snprintf(message, sizeof(message), "Unused variable '%s'", name);
    snprintf(suggestion, sizeof(suggestion), "Remove the variable declaration or use it in your code");
    
    semantic_warning_with_suggestion(analyzer, message, suggestion, line, column);
}

void semantic_warning_unreachable_code(SemanticAnalyzer* analyzer, int line, int column) {
    char message[256];
    char suggestion[256];
    
    snprintf(message, sizeof(message), "Unreachable code detected");
    snprintf(suggestion, sizeof(suggestion), "This code will never be executed. Consider removing it or fixing the control flow");
    
    semantic_warning_with_suggestion(analyzer, message, suggestion, line, column);
}

void semantic_warning_type_conversion(SemanticAnalyzer* analyzer, DataType from_type, DataType to_type, int line, int column) {
    char message[256];
    char suggestion[256];
    
    snprintf(message, sizeof(message), "Implicit conversion from %s to %s", 
             data_type_to_string(from_type), data_type_to_string(to_type));
    snprintf(suggestion, sizeof(suggestion), "Consider using explicit conversion to make your intent clear");
    
    semantic_warning_with_suggestion(analyzer, message, suggestion, line, column);
}

void semantic_warning_performance(SemanticAnalyzer* analyzer, const char* issue, int line, int column) {
    char message[256];
    char suggestion[256];
    
    snprintf(message, sizeof(message), "Performance warning: %s", issue);
    snprintf(suggestion, sizeof(suggestion), "Consider optimizing this code for better performance");
    
    semantic_warning_with_suggestion(analyzer, message, suggestion, line, column);
} 