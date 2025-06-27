#ifndef LEXER_H
#define LEXER_H

#include "common.h"

typedef enum {
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    
    TOKEN_FUNC,
    TOKEN_LET,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_RETURN,
    TOKEN_PRINT,
    
    TOKEN_INT,
    TOKEN_BOOL,
    
    // Operators
    TOKEN_PLUS,      // +
    TOKEN_MINUS,     // -
    TOKEN_STAR,      // *
    TOKEN_SLASH,     // /
    TOKEN_PERCENT,   // %
    TOKEN_BANG,      // !
    TOKEN_ASSIGN,    // =
    TOKEN_EQ,        // ==
    TOKEN_NE,        // !=
    TOKEN_LT,        // <
    TOKEN_LE,        // <=
    TOKEN_GT,        // >
    TOKEN_GE,        // >=
    TOKEN_AND,       // &&
    TOKEN_OR,        // ||
    
    // Punctuation
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }
    TOKEN_SEMICOLON, // ;
    TOKEN_COLON,     // :
    TOKEN_COMMA,     // ,
    TOKEN_ARROW,     // ->
    
    TOKEN_EOF,
    TOKEN_ERROR,
    
    TOKEN_TRUE,      // true
    TOKEN_FALSE,     // false
} TokenType;

typedef struct {
    TokenType type;
    char* lexeme;
    int line;
    int column;
    union {
        int64_t number_value;
        bool bool_value;
    } literal;
} Token;

typedef struct {
    const char* source;
    size_t start;
    size_t current;
    int line;
    int column;
    Error* error;
} Lexer;

Lexer* lexer_create(const char* source, Error* error);
void lexer_destroy(Lexer* lexer);
Token lexer_next_token(Lexer* lexer);
Token lexer_peek_token(Lexer* lexer);
bool lexer_is_at_end(Lexer* lexer);

const char* token_type_to_string(TokenType type);
void token_print(const Token* token);
void token_destroy(Token* token);

#endif