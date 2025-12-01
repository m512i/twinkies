#include "frontend/lexer/lexer.h"
#include "common/flags.h"
#include <ctype.h>

typedef struct
{
    const char *keyword;
    TLTokenType type;
} Keyword;

/*
Note: This array is kept for potential future optimization but currently unused
// as identifier_type() uses direct string comparisons for simplicity
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
}; */

Lexer *lexer_create(const char *source, Error *error)
{
    Lexer *lexer = safe_malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->start = 0;
    lexer->current = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->error = error;
    return lexer;
}

void lexer_destroy(Lexer *lexer)
{
    safe_free(lexer);
}

static bool is_at_end(Lexer *lexer)
{
    return lexer->source[lexer->current] == '\0';
}

static char advance(Lexer *lexer)
{
    char c = lexer->source[lexer->current++];
    if (c == '\n')
    {
        lexer->line++;
        lexer->column = 1;
    }
    else
    {
        lexer->column++;
    }
    return c;
}

static char peek(Lexer *lexer)
{
    return lexer->source[lexer->current];
}

static char peek_next(Lexer *lexer)
{
    if (is_at_end(lexer))
        return '\0';
    return lexer->source[lexer->current + 1];
}

static bool match(Lexer *lexer, char expected)
{
    if (is_at_end(lexer))
        return false;
    if (lexer->source[lexer->current] != expected)
        return false;

    lexer->current++;
    lexer->column++;
    return true;
}

static Token make_token(Lexer *lexer, TLTokenType type);

static void skip_whitespace(Lexer *lexer)
{
    while (!is_at_end(lexer))
    {
        char c = peek(lexer);
        if (c == ' ' || c == '\r' || c == '\t' || c == '\n')
        {
            advance(lexer);
        }
        else if (c == '/' && peek_next(lexer) == '/')
        {
            advance(lexer);
            advance(lexer);
            while (peek(lexer) != '\n' && !is_at_end(lexer))
            {
                advance(lexer);
            }
        }
        else if (c == '/' && peek_next(lexer) == '*')
        {
            advance(lexer);
            advance(lexer);
            
            while (!is_at_end(lexer))
            {
                if (peek(lexer) == '*' && peek_next(lexer) == '/')
                {
                    advance(lexer);
                    advance(lexer);
                    break;
                }
                advance(lexer);
            }
            
            if (is_at_end(lexer) && lexer->error)
            {
                error_set_with_suggestion(lexer->error, ERROR_LEXER, 
                    "Unterminated block comment", 
                    "Add closing */ to terminate the block comment", 
                    lexer->line, lexer->column);
            }
        }
        else
        {
            break;
        }
    }
}

static Token handle_preprocessor_directive(Lexer *lexer)
{
    advance(lexer);

    while (!is_at_end(lexer) && (peek(lexer) == ' ' || peek(lexer) == '\t'))
    {
        advance(lexer);
    }

    lexer->start = lexer->current;

    if (debug_enabled)
    {
        printf("[DEBUG] After #, current char: '%c', remaining: %s\n",
               peek(lexer), lexer->source + lexer->current);
        fflush(stdout);
    }

    if (debug_enabled)
    {
        printf("[DEBUG] Checking for 'include': current=%zu, strlen=%zu, remaining='%.7s'\n",
               lexer->current, strlen(lexer->source), lexer->source + lexer->current);
        fflush(stdout);
    }

    if (!is_at_end(lexer) &&
        (lexer->current + 7) <= strlen(lexer->source))
    {
        char buffer[8];
        strncpy(buffer, lexer->source + lexer->current, 7);
        buffer[7] = '\0';

        if (debug_enabled)
        {
            printf("[DEBUG] Comparing '%s' with 'include'\n", buffer);
            fflush(stdout);
        }

        if (strcmp(buffer, "include") == 0)
        {
            if (debug_enabled)
            {
                printf("[DEBUG] Found 'include', returning TOKEN_INCLUDE\n");
                fflush(stdout);
            }
            for (int i = 0; i < 7; i++)
            {
                advance(lexer);
            }
            while (!is_at_end(lexer) && (peek(lexer) == ' ' || peek(lexer) == '\t'))
            {
                advance(lexer);
            }
            lexer->start = lexer->current;
            return make_token(lexer, TOKEN_INCLUDE);
        }
    }

    if (debug_enabled)
    {
        printf("[DEBUG] No 'include' found, returning TOKEN_HASH\n");
        fflush(stdout);
    }

    return make_token(lexer, TOKEN_HASH);
}

static Token make_token(Lexer *lexer, TLTokenType type)
{
    Token token;
    token.type = type;

    size_t token_length = lexer->current - lexer->start;

    token.lexeme = safe_malloc(token_length + 1);

    strncpy(token.lexeme, lexer->source + lexer->start, token_length);
    token.lexeme[token_length] = '\0';

    token.line = lexer->line;
    token.column = lexer->column - (lexer->current - lexer->start);
    if (type == TOKEN_TRUE)
    {
        token.literal.bool_value = true;
    }
    else if (type == TOKEN_FALSE)
    {
        token.literal.bool_value = false;
    }
    return token;
}

static Token error_token(Lexer *lexer, const char *message)
{
    Token token;
    token.type = TOKEN_ERROR;
    token.lexeme = string_copy(message);
    token.line = lexer->line;
    token.column = lexer->column;

    if (lexer->error)
    {
        char suggestion[256] = "";

        if (strstr(message, "Unexpected character"))
        {
            strcpy(suggestion, "Check for valid characters: letters, digits, operators, and punctuation");
        }
        else if (strstr(message, "Unterminated string"))
        {
            strcpy(suggestion, "Add closing double quote (\") to terminate the string literal");
        }
        else if (strstr(message, "Invalid number"))
        {
            strcpy(suggestion, "Use only digits (0-9) for integer literals");
        }
        else if (strstr(message, "Invalid identifier"))
        {
            strcpy(suggestion, "Identifiers must start with a letter or underscore, followed by letters, digits, or underscores");
        }

        if (suggestion[0] != '\0')
        {
            error_set_with_suggestion(lexer->error, ERROR_LEXER, message, suggestion, token.line, token.column);
        }
        else
        {
            error_set(lexer->error, ERROR_LEXER, message, token.line, token.column);
        }
    }

    return token;
}

static TLTokenType identifier_type(const char *lexeme)
{
    if (strcmp(lexeme, "func") == 0)
        return TOKEN_FUNC;
    if (strcmp(lexeme, "let") == 0)
        return TOKEN_LET;
    if (strcmp(lexeme, "if") == 0)
        return TOKEN_IF;
    if (strcmp(lexeme, "else") == 0)
        return TOKEN_ELSE;
    if (strcmp(lexeme, "while") == 0)
        return TOKEN_WHILE;
    if (strcmp(lexeme, "break") == 0)
        return TOKEN_BREAK;
    if (strcmp(lexeme, "continue") == 0)
        return TOKEN_CONTINUE;
    if (strcmp(lexeme, "return") == 0)
        return TOKEN_RETURN;
    if (strcmp(lexeme, "print") == 0)
        return TOKEN_PRINT;
    if (strcmp(lexeme, "extern") == 0)
        return TOKEN_EXTERN;
    if (strcmp(lexeme, "from") == 0)
        return TOKEN_FROM;
    if (strcmp(lexeme, "int") == 0)
        return TOKEN_INT;
    if (strcmp(lexeme, "int8") == 0)
        return TOKEN_INT8;
    if (strcmp(lexeme, "int16") == 0)
        return TOKEN_INT16;
    if (strcmp(lexeme, "int32") == 0)
        return TOKEN_INT32;
    if (strcmp(lexeme, "int64") == 0)
        return TOKEN_INT64;
    if (strcmp(lexeme, "bool") == 0)
        return TOKEN_BOOL;
    if (strcmp(lexeme, "float") == 0)
        return TOKEN_FLOAT;
    if (strcmp(lexeme, "double") == 0)
        return TOKEN_DOUBLE;
    if (strcmp(lexeme, "string") == 0)
        return TOKEN_STRING_TYPE;
    if (strcmp(lexeme, "void") == 0)
        return TOKEN_VOID;
    if (strcmp(lexeme, "true") == 0)
        return TOKEN_TRUE;
    if (strcmp(lexeme, "false") == 0)
        return TOKEN_FALSE;
    if (strcmp(lexeme, "null") == 0)
        return TOKEN_NULL;
    if (strcmp(lexeme, "asm") == 0)
        return TOKEN_ASM;
    if (strcmp(lexeme, "volatile") == 0)
        return TOKEN_VOLATILE;
    return TOKEN_IDENTIFIER;
}

static bool is_alpha(char c)
{
    return isalpha(c) || c == '_';
}

static bool is_alphanumeric(char c)
{
    return isalpha(c) || isdigit(c) || c == '_';
}

static Token identifier(Lexer *lexer)
{
    while (is_alphanumeric(peek(lexer)))
    {
        advance(lexer);
    }

    Token token = make_token(lexer, TOKEN_IDENTIFIER);
    token.type = identifier_type(token.lexeme);
    if (token.type == TOKEN_TRUE)
    {
        token.literal.bool_value = true;
    }
    else if (token.type == TOKEN_FALSE)
    {
        token.literal.bool_value = false;
    }
    return token;
}

static Token number(Lexer *lexer)
{
    while (isdigit(peek(lexer)))
    {
        advance(lexer);
    }

    bool is_float = false;
    if (peek(lexer) == '.' && isdigit(peek_next(lexer)))
    {
        is_float = true;
        advance(lexer);
        while (isdigit(peek(lexer)))
        {
            advance(lexer);
        }
    }

    if ((peek(lexer) == 'e' || peek(lexer) == 'E'))
    {
        is_float = true;
        advance(lexer);
        if (peek(lexer) == '+' || peek(lexer) == '-')
        {
            advance(lexer);
        }
        if (!isdigit(peek(lexer)))
        {
            return error_token(lexer, "Malformed scientific notation");
        }
        while (isdigit(peek(lexer)))
        {
            advance(lexer);
        }
    }

    Token token = make_token(lexer, TOKEN_NUMBER);
    if (is_float)
    {
        token.literal.float_value = atof(token.lexeme);
    }
    else
    {
        token.literal.number_value = atoll(token.lexeme);
    }
    return token;
}

static Token string_literal(Lexer *lexer)
{
    while (peek(lexer) != '"' && !is_at_end(lexer))
    {
        if (peek(lexer) == '\n')
        {
            lexer->line++;
        }
        advance(lexer);
    }

    if (is_at_end(lexer))
    {
        return error_token(lexer, "Unterminated string");
    }

    advance(lexer);

    Token token = make_token(lexer, TOKEN_STRING_LITERAL);

    size_t length = lexer->current - lexer->start - 2;
    char *string_content = safe_malloc(length + 1);
    
    size_t src_pos = lexer->start + 1;
    size_t dst_pos = 0;
    
    while (src_pos < lexer->current - 1) 
    {
        if (lexer->source[src_pos] == '\\' && src_pos + 1 < lexer->current - 1)
        {
            char next_char = lexer->source[src_pos + 1];
            switch (next_char)
            {
                case 'n':
                    string_content[dst_pos++] = '\n';
                    src_pos += 2;
                    break;
                case 't':
                    string_content[dst_pos++] = '\t';
                    src_pos += 2;
                    break;
                case 'r':
                    string_content[dst_pos++] = '\r';
                    src_pos += 2;
                    break;
                case '\\':
                    string_content[dst_pos++] = '\\';
                    src_pos += 2;
                    break;
                case '"':
                    string_content[dst_pos++] = '"';
                    src_pos += 2;
                    break;
                default:
                    // Unknown escape sequence, treat as literal
                    string_content[dst_pos++] = lexer->source[src_pos++];
                    break;
            }
        }
        else
        {
            string_content[dst_pos++] = lexer->source[src_pos++];
        }
    }
    
    string_content[dst_pos] = '\0';

    token.literal.string_value = string_content;
    return token;
}

Token lexer_next_token(Lexer *lexer)
{
    skip_whitespace(lexer);

    lexer->start = lexer->current;

    if (is_at_end(lexer))
    {
        return make_token(lexer, TOKEN_EOF);
    }

    char c = advance(lexer);

    if (is_alpha(c))
    {
        return identifier(lexer);
    }

    if (isdigit(c))
    {
        return number(lexer);
    }

    switch (c)
    {
    case '(':
        return make_token(lexer, TOKEN_LPAREN);
    case ')':
        return make_token(lexer, TOKEN_RPAREN);
    case '{':
        return make_token(lexer, TOKEN_LBRACE);
    case '}':
        return make_token(lexer, TOKEN_RBRACE);
    case '[':
        return make_token(lexer, TOKEN_LBRACKET);
    case ']':
        return make_token(lexer, TOKEN_RBRACKET);
    case ';':
        return make_token(lexer, TOKEN_SEMICOLON);
    case ':':
        return make_token(lexer, TOKEN_COLON);
    case ',':
        return make_token(lexer, TOKEN_COMMA);
    case '+':
        return make_token(lexer, TOKEN_PLUS);
    case '-':
        if (match(lexer, '>'))
        {
            return make_token(lexer, TOKEN_ARROW);
        }
        return make_token(lexer, TOKEN_MINUS);
    case '*':
        return make_token(lexer, TOKEN_STAR);
    case '/':
        return make_token(lexer, TOKEN_SLASH);
    case '%':
        return make_token(lexer, TOKEN_PERCENT);
    case '!':
        if (match(lexer, '='))
        {
            return make_token(lexer, TOKEN_NE);
        }
        return make_token(lexer, TOKEN_BANG);
    case '=':
        if (match(lexer, '='))
        {
            return make_token(lexer, TOKEN_EQ);
        }
        return make_token(lexer, TOKEN_ASSIGN);
    case '<':
        if (match(lexer, '='))
        {
            return make_token(lexer, TOKEN_LE);
        }
        return make_token(lexer, TOKEN_LT);
    case '>':
        if (match(lexer, '='))
        {
            return make_token(lexer, TOKEN_GE);
        }
        return make_token(lexer, TOKEN_GT);
    case '&':
        if (match(lexer, '&'))
        {
            return make_token(lexer, TOKEN_AND);
        }
        break;
    case '|':
        if (match(lexer, '|'))
        {
            return make_token(lexer, TOKEN_OR);
        }
        break;
    case '"':
        return string_literal(lexer);
    case '#':
        return handle_preprocessor_directive(lexer);
    }

    return error_token(lexer, "Unexpected character");
}

Token lexer_peek_token(Lexer *lexer)
{
    size_t saved_start = lexer->start;
    size_t saved_current = lexer->current;
    int saved_line = lexer->line;
    int saved_column = lexer->column;
    Error *saved_error = lexer->error;

    Token token = lexer_next_token(lexer);

    lexer->start = saved_start;
    lexer->current = saved_current;
    lexer->line = saved_line;
    lexer->column = saved_column;
    lexer->error = saved_error;

    return token;
}

bool lexer_is_at_end(Lexer *lexer)
{
    return is_at_end(lexer);
}

const char *token_type_to_string(TLTokenType type)
{
    switch (type)
    {
    case TOKEN_NUMBER:
        return "NUMBER";
    case TOKEN_IDENTIFIER:
        return "IDENTIFIER";
    case TOKEN_FUNC:
        return "FUNC";
    case TOKEN_LET:
        return "LET";
    case TOKEN_IF:
        return "IF";
    case TOKEN_ELSE:
        return "ELSE";
    case TOKEN_WHILE:
        return "WHILE";
    case TOKEN_BREAK:
        return "BREAK";
    case TOKEN_CONTINUE:
        return "CONTINUE";
    case TOKEN_RETURN:
        return "RETURN";
    case TOKEN_PRINT:
        return "PRINT";
    case TOKEN_INT:
        return "INT";
    case TOKEN_INT8:
        return "INT8";
    case TOKEN_INT16:
        return "INT16";
    case TOKEN_INT32:
        return "INT32";
    case TOKEN_INT64:
        return "INT64";
    case TOKEN_BOOL:
        return "BOOL";
    case TOKEN_FLOAT:
        return "FLOAT";
    case TOKEN_DOUBLE:
        return "DOUBLE";
    case TOKEN_PLUS:
        return "PLUS";
    case TOKEN_MINUS:
        return "MINUS";
    case TOKEN_STAR:
        return "STAR";
    case TOKEN_SLASH:
        return "SLASH";
    case TOKEN_PERCENT:
        return "PERCENT";
    case TOKEN_BANG:
        return "BANG";
    case TOKEN_ASSIGN:
        return "ASSIGN";
    case TOKEN_EQ:
        return "EQ";
    case TOKEN_NE:
        return "NE";
    case TOKEN_LT:
        return "LT";
    case TOKEN_LE:
        return "LE";
    case TOKEN_GT:
        return "GT";
    case TOKEN_GE:
        return "GE";
    case TOKEN_AND:
        return "AND";
    case TOKEN_OR:
        return "OR";
    case TOKEN_LPAREN:
        return "LPAREN";
    case TOKEN_RPAREN:
        return "RPAREN";
    case TOKEN_LBRACE:
        return "LBRACE";
    case TOKEN_RBRACE:
        return "RBRACE";
    case TOKEN_LBRACKET:
        return "LBRACKET";
    case TOKEN_RBRACKET:
        return "RBRACKET";
    case TOKEN_SEMICOLON:
        return "SEMICOLON";
    case TOKEN_COLON:
        return "COLON";
    case TOKEN_COMMA:
        return "COMMA";
    case TOKEN_ARROW:
        return "ARROW";
    case TOKEN_EOF:
        return "EOF";
    case TOKEN_ERROR:
        return "ERROR";
    case TOKEN_TRUE:
        return "TRUE";
    case TOKEN_FALSE:
        return "FALSE";
    case TOKEN_NULL:
        return "NULL";
    case TOKEN_STRING:
        return "STRING";
    case TOKEN_STRING_LITERAL:
        return "STRING_LITERAL";
    case TOKEN_STRING_TYPE:
        return "STRING_TYPE";
    case TOKEN_VOID:
        return "VOID";
    case TOKEN_INCLUDE:
        return "INCLUDE";
    case TOKEN_HASH:
        return "HASH";
    case TOKEN_ASM:
        return "ASM";
    case TOKEN_VOLATILE:
        return "VOLATILE";
    default:
        return "UNKNOWN";
    }
}

void token_print(const Token *token)
{
    printf("Token{type: %s, lexeme: '%s', line: %d, column: %d",
           token_type_to_string(token->type), token->lexeme, token->line, token->column);

    if (token->type == TOKEN_NUMBER)
    {
        if (strchr(token->lexeme, '.') != NULL)
        {
            printf(", value: %f", token->literal.float_value);
        }
        else
        {
            printf(", value: %lld", token->literal.number_value);
        }
    }

    if (token->type == TOKEN_TRUE || token->type == TOKEN_FALSE)
    {
        printf(", value: %s", token->literal.bool_value ? "true" : "false");
    }

    if (token->type == TOKEN_STRING)
    {
        printf(", value: \"%s\"", token->literal.string_value);
    }

    printf("}\n");
}

void token_destroy(Token *token)
{
    if (token->lexeme)
    {
        safe_free(token->lexeme);
        token->lexeme = NULL;
    }

    if (token->type == TOKEN_STRING && token->literal.string_value)
    {
        safe_free(token->literal.string_value);
        token->literal.string_value = NULL;
    }
}