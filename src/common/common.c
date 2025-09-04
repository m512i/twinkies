#include "common.h"
#include <stdarg.h>

extern bool suppress_warnings;

void error_init(Error *error)
{
    error->type = ERROR_NONE;
    error->severity = SEVERITY_ERROR;
    error->message[0] = '\0';
    error->suggestion[0] = '\0';
    error->line = 0;
    error->column = 0;
    error->source_line[0] = '\0';
    error->source_start = 0;
    error->source_end = 0;
}

void error_set(Error *error, ErrorType type, const char *message, int line, int column)
{
    error->type = type;
    error->severity = SEVERITY_ERROR;
    strncpy(error->message, message, sizeof(error->message) - 1);
    error->message[sizeof(error->message) - 1] = '\0';
    error->suggestion[0] = '\0';
    error->line = line;
    error->column = column;
    error->source_start = 0;
    error->source_end = 0;
}

void error_set_with_suggestion(Error *error, ErrorType type, const char *message, const char *suggestion, int line, int column)
{
    error->type = type;
    error->severity = SEVERITY_ERROR;
    strncpy(error->message, message, sizeof(error->message) - 1);
    error->message[sizeof(error->message) - 1] = '\0';
    strncpy(error->suggestion, suggestion, sizeof(error->suggestion) - 1);
    error->suggestion[sizeof(error->suggestion) - 1] = '\0';
    error->line = line;
    error->column = column;
    error->source_start = 0;
    error->source_end = 0;
}

void error_print(const Error *error, const char *filename)
{
    if (!error || error->type == ERROR_NONE)
        return;

    fprintf(stderr, "%s" ANSI_BOLD "%s:%d:%d" ANSI_RESET ": ",
            error->severity == SEVERITY_ERROR ? ANSI_ERROR : error->severity == SEVERITY_WARNING ? ANSI_WARNING
                                                         : error->severity == SEVERITY_INFO      ? ANSI_INFO
                                                                                                 : ANSI_HINT,
            filename, error->line, error->column);

    switch (error->type)
    {
    case ERROR_LEXER:
        fprintf(stderr, ANSI_BOLD "lexical %s" ANSI_RESET ": ",
                error->severity == SEVERITY_WARNING ? "warning" : "error");
        break;
    case ERROR_PARSER:
        fprintf(stderr, ANSI_BOLD "syntax %s" ANSI_RESET ": ",
                error->severity == SEVERITY_WARNING ? "warning" : "error");
        break;
    case ERROR_SEMANTIC:
        fprintf(stderr, ANSI_BOLD "semantic %s" ANSI_RESET ": ",
                error->severity == SEVERITY_WARNING ? "warning" : "error");
        break;
    case ERROR_CODEGEN:
        fprintf(stderr, ANSI_BOLD "code generation %s" ANSI_RESET ": ",
                error->severity == SEVERITY_WARNING ? "warning" : "error");
        break;
    default:
        fprintf(stderr, ANSI_BOLD "unknown %s" ANSI_RESET ": ",
                error->severity == SEVERITY_WARNING ? "warning" : "error");
        break;
    }

    fprintf(stderr, "%s\n", error->message);

    if (error->source_line[0] != '\0')
    {
        fprintf(stderr, "  %s\n", error->source_line);

        if (error->column > 0)
        {
            fprintf(stderr, "  ");
            for (int i = 1; i < error->column; i++)
            {
                fprintf(stderr, " ");
            }
            fprintf(stderr, "%s^" ANSI_RESET "\n",
                    error->severity == SEVERITY_WARNING ? ANSI_WARNING : ANSI_ERROR);
        }
    }

    if (error->suggestion[0] != '\0')
    {
        fprintf(stderr, "  %sHint: %s%s\n", ANSI_HINT, error->suggestion, ANSI_RESET);
    }
}

ErrorContext *error_context_create(const char *filename, const char *source_code)
{
    ErrorContext *context = safe_malloc(sizeof(ErrorContext));
    context->errors = safe_malloc(16 * sizeof(Error));
    context->count = 0;
    context->capacity = 16;
    context->source_code = string_copy(source_code);
    context->filename = string_copy(filename);
    return context;
}

void error_context_destroy(ErrorContext *context)
{
    if (!context)
        return;
    safe_free(context->errors);
    safe_free(context->source_code);
    safe_free(context->filename);
    safe_free(context);
}

void error_context_add_error(ErrorContext *context, ErrorType type, ErrorSeverity severity,
                             const char *message, const char *suggestion, int line, int column)
{
    if (!context)
        return;

    if (context->count >= context->capacity)
    {
        context->capacity *= 2;
        context->errors = safe_realloc(context->errors, context->capacity * sizeof(Error));
    }

    Error *error = &context->errors[context->count++];
    error_init(error);
    error->type = type;
    error->severity = severity;
    strncpy(error->message, message, sizeof(error->message) - 1);
    error->message[sizeof(error->message) - 1] = '\0';

    if (suggestion)
    {
        strncpy(error->suggestion, suggestion, sizeof(error->suggestion) - 1);
        error->suggestion[sizeof(error->suggestion) - 1] = '\0';
    }

    error->line = line;
    error->column = column;

    char *source_line = get_source_line(context->source_code, line);
    if (source_line)
    {
        strncpy(error->source_line, source_line, sizeof(error->source_line) - 1);
        error->source_line[sizeof(error->source_line) - 1] = '\0';
        safe_free(source_line);
    }
}

void error_context_print_all(ErrorContext *context)
{
    if (!context || context->count == 0)
        return;

    fprintf(stderr, "\n");

    size_t error_count = 0;
    size_t warning_count = 0;

    for (size_t i = 0; i < context->count; i++)
    {
        if (context->errors[i].severity == SEVERITY_ERROR)
        {
            error_count++;
            error_print(&context->errors[i], context->filename);
        }
        else if (context->errors[i].severity == SEVERITY_WARNING)
        {
            warning_count++;
            extern bool suppress_warnings;
            if (!suppress_warnings)
            {
                error_print(&context->errors[i], context->filename);
            }
        }
        else
        {
            error_print(&context->errors[i], context->filename);
        }
        if (i < context->count - 1)
        {
            fprintf(stderr, "\n");
        }
    }

    if (error_count > 0)
    {
        fprintf(stderr, "\n%s" ANSI_BOLD "Compilation failed with %zu error(s)" ANSI_RESET,
                ANSI_ERROR, error_count);
        if (warning_count > 0 && !suppress_warnings)
        {
            fprintf(stderr, " and %zu warning(s)", warning_count);
        }
        fprintf(stderr, "\n");
    }
    else if (warning_count > 0 && !suppress_warnings)
    {
        fprintf(stderr, "\n%s" ANSI_BOLD "Compilation completed with %zu warning(s)" ANSI_RESET "\n",
                ANSI_WARNING, warning_count);
    }
}

bool error_context_has_errors(ErrorContext *context)
{
    if (!context)
        return false;

    for (size_t i = 0; i < context->count; i++)
    {
        if (context->errors[i].severity == SEVERITY_ERROR)
        {
            return true;
        }
    }
    return false;
}

void error_context_print_source_line(ErrorContext *context, int line, int column, int start, int end)
{
    (void)column;
    if (!context || !context->source_code)
        return;

    char *source_line = get_source_line(context->source_code, line);
    if (!source_line)
        return;

    fprintf(stderr, "  %s\n", source_line);

    fprintf(stderr, "  ");
    for (int i = 1; i < start; i++)
    {
        fprintf(stderr, " ");
    }
    fprintf(stderr, "%s", ANSI_ERROR);
    for (int i = start; i <= end && i <= (int)strlen(source_line); i++)
    {
        fprintf(stderr, "^");
    }
    fprintf(stderr, "%s\n", ANSI_RESET);

    safe_free(source_line);
}

void print_fatal_error(const char *program_name, const char *message)
{
    fprintf(stderr, "%s: %s" ANSI_BOLD "fatal error" ANSI_RESET ": %s\n",
            program_name, ANSI_ERROR, message);
    fflush(stderr);
}

void print_error(const char *program_name, const char *message)
{
    fprintf(stderr, "%s: %s" ANSI_BOLD "error" ANSI_RESET ": %s\n",
            program_name, ANSI_ERROR, message);
    fflush(stderr);
}

void print_warning(const char *program_name, const char *message)
{
    fprintf(stderr, "%s: %s" ANSI_BOLD "warning" ANSI_RESET ": %s\n",
            program_name, ANSI_WARNING, message);
    fflush(stderr);
}

void print_info(const char *program_name, const char *message)
{
    fprintf(stderr, "%s: %s" ANSI_BOLD "info" ANSI_RESET ": %s\n",
            program_name, ANSI_INFO, message);
    fflush(stderr);
}

void print_hint(const char *program_name, const char *message)
{
    fprintf(stderr, "%s: %s" ANSI_BOLD "hint" ANSI_RESET ": %s\n",
            program_name, ANSI_HINT, message);
    fflush(stderr);
}

char *get_source_line(const char *source_code, int line)
{
    if (!source_code || line <= 0)
        return NULL;

    int current_line = 1;
    const char *start = source_code;
    const char *end = source_code;

    while (*end && current_line < line)
    {
        if (*end == '\n')
        {
            current_line++;
            if (current_line == line)
            {
                start = end + 1;
            }
        }
        end++;
    }

    if (current_line != line)
        return NULL;

    while (*end && *end != '\n')
    {
        end++;
    }

    size_t length = end - start;
    char *result = safe_malloc(length + 1);
    strncpy(result, start, length);
    result[length] = '\0';

    return result;
}

void highlight_source_range(char *dest, const char *source, int start, int end, size_t dest_size)
{
    if (!dest || !source || start < 0 || end < start)
        return;

    size_t source_len = strlen(source);
    size_t pos = 0;

    for (size_t i = 0; i < source_len && pos < dest_size - 1; i++)
    {
        if (i == (size_t)start)
        {
            if (pos + 5 < dest_size)
            {
                strcpy(dest + pos, ANSI_ERROR);
                pos += 5;
            }
        }

        if (pos < dest_size - 1)
        {
            dest[pos++] = source[i];
        }

        if (i == (size_t)end)
        {
            if (pos + 4 < dest_size)
            {
                strcpy(dest + pos, ANSI_RESET);
                pos += 4;
            }
        }
    }

    dest[pos] = '\0';
}

size_t total_memory_allocated = 0;
size_t total_memory_freed = 0;
size_t total_allocations = 0;
size_t total_frees = 0;

void *safe_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (!ptr)
    {
        print_fatal_error("compiler", "memory allocation failed");
        exit(1);
    }
    total_memory_allocated += size;
    total_allocations++;
    return ptr;
}

void *safe_realloc(void *ptr, size_t size)
{
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr)
    {
        print_fatal_error("compiler", "memory reallocation failed");
        exit(1);
    }
    total_memory_allocated += size;
    total_allocations++;
    return new_ptr;
}

void safe_free(void *ptr)
{
    if (ptr)
    {
        free(ptr);
        total_frees++;
    }
}

void print_memory_usage_stats(void)
{
    printf("Memory usage statistics:\n");
    printf("  Total allocations: %zu\n", total_allocations);
    printf("  Total frees:       %zu\n", total_frees);
    printf("  Total allocated:   %zu bytes\n", total_memory_allocated);
    printf("  Total freed:       %zu bytes\n", total_memory_freed);
    printf("  Net allocated:     %zu bytes\n", total_memory_allocated - total_memory_freed);
    fflush(stdout);
}

char *string_copy(const char *str)
{
    if (!str)
        return NULL;
    size_t len = strlen(str);
    char *copy = safe_malloc(len + 1);
    strcpy(copy, str);
    return copy;
}

char *string_concat(const char *str1, const char *str2)
{
    if (!str1 || !str2)
        return NULL;
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    char *result = safe_malloc(len1 + len2 + 1);
    strcpy(result, str1);
    strcat(result, str2);
    return result;
}

bool string_equal(const char *str1, const char *str2)
{
    if (!str1 || !str2)
        return str1 == str2;
    return strcmp(str1, str2) == 0;
}

void array_init(DynamicArray *array, size_t initial_capacity)
{
    array->data = safe_malloc(initial_capacity * sizeof(void *));
    array->size = 0;
    array->capacity = initial_capacity;
}

void array_push(DynamicArray *array, void *item)
{
    if (array->size >= array->capacity)
    {
        array->capacity *= 2;
        array->data = safe_realloc(array->data, array->capacity * sizeof(void *));
    }
    array->data[array->size++] = item;
}

void *array_get(const DynamicArray *array, size_t index)
{
    if (index >= array->size)
        return NULL;
    return array->data[index];
}

void array_set(DynamicArray *array, size_t index, void *item)
{
    if (index < array->size)
    {
        array->data[index] = item;
    }
}

void array_free(DynamicArray *array)
{
    safe_free(array->data);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
}

static size_t hash_function(const char *key, size_t capacity)
{
    size_t hash = 5381;
    int c;
    while ((c = *key++))
    {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % capacity;
}

HashTable *hashtable_create(size_t initial_capacity)
{
    HashTable *table = safe_malloc(sizeof(HashTable));
    table->buckets = safe_malloc(initial_capacity * sizeof(HashTableEntry *));
    for (size_t i = 0; i < initial_capacity; i++)
    {
        table->buckets[i] = NULL;
    }
    table->size = 0;
    table->capacity = initial_capacity;
    return table;
}

void hashtable_destroy(HashTable *table)
{
    if (!table)
        return;

    for (size_t i = 0; i < table->capacity; i++)
    {
        HashTableEntry *entry = table->buckets[i];
        while (entry)
        {
            HashTableEntry *next = entry->next;
            safe_free(entry->key);
            safe_free(entry);
            entry = next;
        }
    }
    safe_free(table->buckets);
    safe_free(table);
}

void *hashtable_get(HashTable *table, const char *key)
{
    if (!table || !key)
        return NULL;

    size_t hash = hash_function(key, table->capacity);
    HashTableEntry *entry = table->buckets[hash];

    while (entry)
    {
        if (string_equal(entry->key, key))
        {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

void hashtable_put(HashTable *table, const char *key, void *value)
{
    if (!table || !key)
        return;

    size_t hash = hash_function(key, table->capacity);
    HashTableEntry *entry = table->buckets[hash];

    while (entry)
    {
        if (string_equal(entry->key, key))
        {
            entry->value = value;
            return;
        }
        entry = entry->next;
    }

    HashTableEntry *new_entry = safe_malloc(sizeof(HashTableEntry));
    new_entry->key = string_copy(key);
    new_entry->value = value;
    new_entry->next = table->buckets[hash];
    table->buckets[hash] = new_entry;
    table->size++;
}

bool hashtable_contains(HashTable *table, const char *key)
{
    return hashtable_get(table, key) != NULL;
}

void hashtable_remove(HashTable *table, const char *key)
{
    if (!table || !key)
        return;

    size_t hash = hash_function(key, table->capacity);
    HashTableEntry *entry = table->buckets[hash];
    HashTableEntry *prev = NULL;

    while (entry)
    {
        if (string_equal(entry->key, key))
        {
            if (prev)
            {
                prev->next = entry->next;
            }
            else
            {
                table->buckets[hash] = entry->next;
            }
            safe_free(entry->key);
            safe_free(entry);
            table->size--;
            return;
        }
        prev = entry;
        entry = entry->next;
    }
}