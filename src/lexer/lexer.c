#include "../include/lexer.h"
#include <ctype.h>

typedef struct {
    const char* keyword;
    TokenType type;
} Keyword;

static const Keyword keywords[] = {
    {"func", TOKEN_FUNC},
    {"let", TOKEN_LET},
    {"if", TOKEN_IF},
    {"else", TOKEN_ELSE},
    {"while", TOKEN_WHILE},
    {"return", TOKEN_RETURN},
    {"print", TOKEN_PRINT},
    {"int", TOKEN_INT},
    {"bool", TOKEN_BOOL},
    {"true", TOKEN_TRUE},
    {"false", TOKEN_FALSE},
    {NULL, TOKEN_ERROR}
};

Lexer* lexer_create(const char* source, Error* error) {
    Lexer* lexer = safe_malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->start = 0;
    lexer->current = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->error = error;
    return lexer;
}

void lexer_destroy(Lexer* lexer) {
    safe_free(lexer);
}

static bool is_at_end(Lexer* lexer) {
    return lexer->source[lexer->current] == '\0';
}

static char advance(Lexer* lexer) {
    char c = lexer->source[lexer->current++];
    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    return c;
}

static char peek(Lexer* lexer) {
    return lexer->source[lexer->current];
}

static char peek_next(Lexer* lexer) {
    if (is_at_end(lexer)) return '\0';
    return lexer->source[lexer->current + 1];
}

static bool match(Lexer* lexer, char expected) {
    if (is_at_end(lexer)) return false;
    if (lexer->source[lexer->current] != expected) return false;
    
    lexer->current++;
    lexer->column++;
    return true;
}

static void skip_whitespace(Lexer* lexer) {
    while (!is_at_end(lexer)) {
        char c = peek(lexer);
        if (c == ' ' || c == '\r' || c == '\t' || c == '\n') {
            advance(lexer);
        } else if (c == '/' && peek_next(lexer) == '/') {
            // Skip comment until end of line
            advance(lexer); // skip first '/'
            advance(lexer); // skip second '/'
            while (peek(lexer) != '\n' && !is_at_end(lexer)) {
                advance(lexer);
            }
        } else {
            break;
        }
    }
}

static Token make_token(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    
    size_t token_length = lexer->current - lexer->start;
    
    token.lexeme = safe_malloc(token_length + 1);
    
    strncpy(token.lexeme, lexer->source + lexer->start, token_length);
    token.lexeme[token_length] = '\0';
    
    token.line = lexer->line;
    token.column = lexer->column - (lexer->current - lexer->start);
    if (type == TOKEN_TRUE) {
        token.literal.bool_value = true;
    } else if (type == TOKEN_FALSE) {
        token.literal.bool_value = false;
    }
    return token;
}

static Token error_token(Lexer* lexer, const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.lexeme = string_copy(message);
    token.line = lexer->line;
    token.column = lexer->column;
    
    if (lexer->error) {
        char suggestion[256] = "";
        
        // Provide context-specific suggestions
        if (strstr(message, "Unexpected character")) {
            strcpy(suggestion, "Check for valid characters: letters, digits, operators, and punctuation");
        } else if (strstr(message, "Unterminated string")) {
            strcpy(suggestion, "Add closing double quote (\") to terminate the string literal");
        } else if (strstr(message, "Invalid number")) {
            strcpy(suggestion, "Use only digits (0-9) for integer literals");
        } else if (strstr(message, "Invalid identifier")) {
            strcpy(suggestion, "Identifiers must start with a letter or underscore, followed by letters, digits, or underscores");
        }
        
        if (suggestion[0] != '\0') {
            error_set_with_suggestion(lexer->error, ERROR_LEXER, message, suggestion, token.line, token.column);
        } else {
            error_set(lexer->error, ERROR_LEXER, message, token.line, token.column);
        }
    }
    
    return token;
}

static TokenType identifier_type(const char* lexeme) {
    for (int i = 0; keywords[i].keyword != NULL; i++) {
        if (string_equal(lexeme, keywords[i].keyword)) {
            return keywords[i].type;
        }
    }
    return TOKEN_IDENTIFIER;
}

static bool is_alpha(char c) {
    return isalpha(c) || c == '_';
}

static bool is_alphanumeric(char c) {
    return isalpha(c) || isdigit(c) || c == '_';
}

static Token identifier(Lexer* lexer) {
    while (is_alphanumeric(peek(lexer))) {
        advance(lexer);
    }
    
    Token token = make_token(lexer, TOKEN_IDENTIFIER);
    token.type = identifier_type(token.lexeme);
    if (token.type == TOKEN_TRUE) {
        token.literal.bool_value = true;
    } else if (token.type == TOKEN_FALSE) {
        token.literal.bool_value = false;
    }
    return token;
}

static Token number(Lexer* lexer) {
    while (isdigit(peek(lexer))) {
        advance(lexer);
    }
    
    Token token = make_token(lexer, TOKEN_NUMBER);
    token.literal.number_value = atoll(token.lexeme);
    return token;
}

static Token string_literal(Lexer* lexer) {
    while (peek(lexer) != '"' && !is_at_end(lexer)) {
        if (peek(lexer) == '\n') {
            lexer->line++;
        }
        advance(lexer);
    }
    
    if (is_at_end(lexer)) {
        return error_token(lexer, "Unterminated string");
    }
    
    advance(lexer);
    
    Token token = make_token(lexer, TOKEN_IDENTIFIER); 
    return token;
}

Token lexer_next_token(Lexer* lexer) {
    skip_whitespace(lexer);
    
    lexer->start = lexer->current;
    
    if (is_at_end(lexer)) {
        return make_token(lexer, TOKEN_EOF);
    }
    
    char c = advance(lexer);
    
    if (is_alpha(c)) {
        return identifier(lexer);
    }
    
    if (isdigit(c)) {
        return number(lexer);
    }
    
    switch (c) {
        case '(': return make_token(lexer, TOKEN_LPAREN);
        case ')': return make_token(lexer, TOKEN_RPAREN);
        case '{': return make_token(lexer, TOKEN_LBRACE);
        case '}': return make_token(lexer, TOKEN_RBRACE);
        case '[': return make_token(lexer, TOKEN_LBRACKET);
        case ']': return make_token(lexer, TOKEN_RBRACKET);
        case ';': return make_token(lexer, TOKEN_SEMICOLON);
        case ':': return make_token(lexer, TOKEN_COLON);
        case ',': return make_token(lexer, TOKEN_COMMA);
        case '+': return make_token(lexer, TOKEN_PLUS);
        case '-': 
            if (match(lexer, '>')) {
                return make_token(lexer, TOKEN_ARROW);
            }
            return make_token(lexer, TOKEN_MINUS);
        case '*': return make_token(lexer, TOKEN_STAR);
        case '/': return make_token(lexer, TOKEN_SLASH);
        case '%': return make_token(lexer, TOKEN_PERCENT);
        case '!': 
            if (match(lexer, '=')) {
                return make_token(lexer, TOKEN_NE);
            }
            return make_token(lexer, TOKEN_BANG);
        case '=': 
            if (match(lexer, '=')) {
                return make_token(lexer, TOKEN_EQ);
            }
            return make_token(lexer, TOKEN_ASSIGN);
        case '<': 
            if (match(lexer, '=')) {
                return make_token(lexer, TOKEN_LE);
            }
            return make_token(lexer, TOKEN_LT);
        case '>': 
            if (match(lexer, '=')) {
                return make_token(lexer, TOKEN_GE);
            }
            return make_token(lexer, TOKEN_GT);
        case '&': 
            if (match(lexer, '&')) {
                return make_token(lexer, TOKEN_AND);
            }
            break;
        case '|': 
            if (match(lexer, '|')) {
                return make_token(lexer, TOKEN_OR);
            }
            break;
        case '"': return string_literal(lexer);
    }
    
    return error_token(lexer, "Unexpected character");
}

Token lexer_peek_token(Lexer* lexer) {
    size_t saved_start = lexer->start;
    size_t saved_current = lexer->current;
    int saved_line = lexer->line;
    int saved_column = lexer->column;
    
    Token token = lexer_next_token(lexer);
    
    lexer->start = saved_start;
    lexer->current = saved_current;
    lexer->line = saved_line;
    lexer->column = saved_column;
    
    return token;
}

bool lexer_is_at_end(Lexer* lexer) {
    return is_at_end(lexer);
}

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_FUNC: return "FUNC";
        case TOKEN_LET: return "LET";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_PRINT: return "PRINT";
        case TOKEN_INT: return "INT";
        case TOKEN_BOOL: return "BOOL";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_STAR: return "STAR";
        case TOKEN_SLASH: return "SLASH";
        case TOKEN_PERCENT: return "PERCENT";
        case TOKEN_BANG: return "BANG";
        case TOKEN_ASSIGN: return "ASSIGN";
        case TOKEN_EQ: return "EQ";
        case TOKEN_NE: return "NE";
        case TOKEN_LT: return "LT";
        case TOKEN_LE: return "LE";
        case TOKEN_GT: return "GT";
        case TOKEN_GE: return "GE";
        case TOKEN_AND: return "AND";
        case TOKEN_OR: return "OR";
        case TOKEN_LPAREN: return "LPAREN";
        case TOKEN_RPAREN: return "RPAREN";
        case TOKEN_LBRACE: return "LBRACE";
        case TOKEN_RBRACE: return "RBRACE";
        case TOKEN_LBRACKET: return "LBRACKET";
        case TOKEN_RBRACKET: return "RBRACKET";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_COLON: return "COLON";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_ARROW: return "ARROW";
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        case TOKEN_TRUE: return "TRUE";
        case TOKEN_FALSE: return "FALSE";
        default: return "UNKNOWN";
    }
}

void token_print(const Token* token) {
    printf("Token{type: %s, lexeme: '%s', line: %d, column: %d", 
           token_type_to_string(token->type), token->lexeme, token->line, token->column);
    
    if (token->type == TOKEN_NUMBER) {
        printf(", value: %lld", token->literal.number_value);
    }
    
    if (token->type == TOKEN_TRUE || token->type == TOKEN_FALSE) {
        printf(", value: %s", token->literal.bool_value ? "true" : "false");
    }
    
    printf("}\n");
}

void token_destroy(Token* token) {
    if (token->lexeme) {
        safe_free(token->lexeme);
        token->lexeme = NULL;
    }
} 