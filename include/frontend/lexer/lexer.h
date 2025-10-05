#ifndef LEXER_H
#define LEXER_H

#include "common/common.h"

typedef enum
{
    TOKEN_NUMBER,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_STRING_LITERAL,

    TOKEN_FUNC,
    TOKEN_LET,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_WHILE,
    TOKEN_BREAK,
    TOKEN_CONTINUE,
    TOKEN_RETURN,
    TOKEN_PRINT,
    TOKEN_EXTERN,
    TOKEN_FROM,

    TOKEN_INT,
    TOKEN_BOOL,
    TOKEN_FLOAT,
    TOKEN_DOUBLE,
    TOKEN_STRING_TYPE,

    // Operators
    TOKEN_PLUS,    // +
    TOKEN_MINUS,   // -
    TOKEN_STAR,    // *
    TOKEN_SLASH,   // /
    TOKEN_PERCENT, // %
    TOKEN_BANG,    // !
    TOKEN_ASSIGN,  // =
    TOKEN_EQ,      // ==
    TOKEN_NE,      // !=
    TOKEN_LT,      // <
    TOKEN_LE,      // <=
    TOKEN_GT,      // >
    TOKEN_GE,      // >=
    TOKEN_AND,     // &&
    TOKEN_OR,      // ||

    // Punctuation
    TOKEN_LPAREN,    // (
    TOKEN_RPAREN,    // )
    TOKEN_LBRACE,    // {
    TOKEN_RBRACE,    // }
    TOKEN_LBRACKET,  // [
    TOKEN_RBRACKET,  // ]
    TOKEN_SEMICOLON, // ;
    TOKEN_COLON,     // :
    TOKEN_COMMA,     // ,
    TOKEN_ARROW,     // ->

    TOKEN_EOF,
    TOKEN_ERROR,

    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,
    TOKEN_INCLUDE,
    TOKEN_HASH,
} TLTokenType;

typedef struct
{
    TLTokenType type;
    char *lexeme;
    int line;
    int column;
    union
    {
        int64_t number_value;
        bool bool_value;
        double float_value;
        char *string_value;
    } literal;
} Token;

typedef struct
{
    const char *source;
    size_t start;
    size_t current;
    int line;
    int column;
    Error *error;
} Lexer;

Lexer *lexer_create(const char *source, Error *error);
void lexer_destroy(Lexer *lexer);
Token lexer_next_token(Lexer *lexer);
Token lexer_peek_token(Lexer *lexer);
bool lexer_is_at_end(Lexer *lexer);

const char *token_type_to_string(TLTokenType type);
void token_print(const Token *token);
void token_destroy(Token *token);

#endif