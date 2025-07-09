#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BLUE "\033[34m"
#define ANSI_MAGENTA "\033[35m"
#define ANSI_CYAN "\033[36m"
#define ANSI_BOLD "\033[1m"
#define ANSI_UNDERLINE "\033[4m"
#define ANSI_RESET "\033[0m"

#define ANSI_ERROR ANSI_RED ANSI_BOLD
#define ANSI_WARNING ANSI_YELLOW ANSI_BOLD
#define ANSI_INFO ANSI_CYAN
#define ANSI_HINT ANSI_GREEN

typedef enum
{
    ERROR_NONE = 0,
    ERROR_LEXER,
    ERROR_PARSER,
    ERROR_SEMANTIC,
    ERROR_CODEGEN
} ErrorType;

typedef enum
{
    SEVERITY_ERROR,
    SEVERITY_WARNING,
    SEVERITY_INFO,
    SEVERITY_HINT
} ErrorSeverity;

typedef struct
{
    ErrorType type;
    ErrorSeverity severity;
    char message[512];
    char suggestion[256];
    int line;
    int column;
    char source_line[256];
    int source_start;
    int source_end;
} Error;

typedef struct
{
    Error *errors;
    size_t count;
    size_t capacity;
    char *source_code;
    char *filename;
} ErrorContext;

void error_init(Error *error);
void error_set(Error *error, ErrorType type, const char *message, int line, int column);
void error_set_with_suggestion(Error *error, ErrorType type, const char *message, const char *suggestion, int line, int column);
void error_print(const Error *error, const char *filename);

ErrorContext *error_context_create(const char *filename, const char *source_code);
void error_context_destroy(ErrorContext *context);
void error_context_add_error(ErrorContext *context, ErrorType type, ErrorSeverity severity,
                             const char *message, const char *suggestion, int line, int column);
void error_context_print_all(ErrorContext *context);
bool error_context_has_errors(ErrorContext *context);
void error_context_print_source_line(ErrorContext *context, int line, int column, int start, int end);

void print_fatal_error(const char *program_name, const char *message);
void print_error(const char *program_name, const char *message);
void print_warning(const char *program_name, const char *message);
void print_info(const char *program_name, const char *message);
void print_hint(const char *program_name, const char *message);

char *get_source_line(const char *source_code, int line);
void highlight_source_range(char *dest, const char *source, int start, int end, size_t dest_size);

void *safe_malloc(size_t size);
void *safe_realloc(void *ptr, size_t size);
void safe_free(void *ptr);

char *string_copy(const char *str);
char *string_concat(const char *str1, const char *str2);
bool string_equal(const char *str1, const char *str2);

typedef struct
{
    void **data;
    size_t size;
    size_t capacity;
} DynamicArray;

void array_init(DynamicArray *array, size_t initial_capacity);
void array_push(DynamicArray *array, void *item);
void *array_get(const DynamicArray *array, size_t index);
void array_set(DynamicArray *array, size_t index, void *item);
void array_free(DynamicArray *array);

typedef struct HashTable HashTable;
typedef struct HashTableEntry HashTableEntry;

struct HashTableEntry
{
    char *key;
    void *value;
    HashTableEntry *next;
};

struct HashTable
{
    HashTableEntry **buckets;
    size_t size;
    size_t capacity;
};

HashTable *hashtable_create(size_t initial_capacity);
void hashtable_destroy(HashTable *table);
void *hashtable_get(HashTable *table, const char *key);
void hashtable_put(HashTable *table, const char *key, void *value);
bool hashtable_contains(HashTable *table, const char *key);
void hashtable_remove(HashTable *table, const char *key);

extern size_t total_memory_allocated;
extern size_t total_memory_freed;
extern size_t total_allocations;
extern size_t total_frees;
void print_memory_usage_stats(void);

#endif