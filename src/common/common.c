#include "../include/common.h"
#include <stdarg.h>

void error_init(Error* error) {
    error->type = ERROR_NONE;
    error->message[0] = '\0';
    error->line = 0;
    error->column = 0;
}

void error_set(Error* error, ErrorType type, const char* message, int line, int column) {
    error->type = type;
    strncpy(error->message, message, sizeof(error->message) - 1);
    error->message[sizeof(error->message) - 1] = '\0';
    error->line = line;
    error->column = column;
}

void error_print(const Error* error, const char* filename) {
    if (!error || error->type == ERROR_NONE) return;
    
    fprintf(stderr, "%s:%d:%d: error: ", filename, error->line, error->column);
    
    switch (error->type) {
        case ERROR_LEXER:
            fprintf(stderr, "lexical error: %s\n", error->message);
            break;
        case ERROR_PARSER:
            fprintf(stderr, "syntax error: %s\n", error->message);
            break;
        case ERROR_SEMANTIC:
            fprintf(stderr, "semantic error: %s\n", error->message);
            break;
        case ERROR_CODEGEN:
            fprintf(stderr, "code generation error: %s\n", error->message);
            break;
        default:
            fprintf(stderr, "unknown error: %s\n", error->message);
            break;
    }
}

void print_fatal_error(const char* program_name, const char* message) {
    fprintf(stderr, "%s: " ANSI_RED ANSI_BOLD "fatal error" ANSI_RESET ": %s\n", program_name, message);
    fflush(stderr);
}

void print_error(const char* program_name, const char* message) {
    fprintf(stderr, "%s: " ANSI_RED "error" ANSI_RESET ": %s\n", program_name, message);
    fflush(stderr);
}

void print_warning(const char* program_name, const char* message) {
    fprintf(stderr, "%s: " ANSI_YELLOW "warning" ANSI_RESET ": %s\n", program_name, message);
    fflush(stderr);
}

void* safe_malloc(size_t size) {
    void* ptr = malloc(size);
    if (!ptr) {
        print_fatal_error("compiler", "memory allocation failed");
        exit(1);
    }
    return ptr;
}

void* safe_realloc(void* ptr, size_t size) {
    void* new_ptr = realloc(ptr, size);
    if (!new_ptr) {
        print_fatal_error("compiler", "memory reallocation failed");
        exit(1);
    }
    return new_ptr;
}

void safe_free(void* ptr) {
    if (ptr) {
        free(ptr);
    }
}

char* string_copy(const char* str) {
    if (!str) return NULL;
    size_t len = strlen(str);
    char* copy = safe_malloc(len + 1);
    strcpy(copy, str);
    return copy;
}

char* string_concat(const char* str1, const char* str2) {
    if (!str1 || !str2) return NULL;
    size_t len1 = strlen(str1);
    size_t len2 = strlen(str2);
    char* result = safe_malloc(len1 + len2 + 1);
    strcpy(result, str1);
    strcat(result, str2);
    return result;
}

bool string_equal(const char* str1, const char* str2) {
    if (!str1 || !str2) return str1 == str2;
    return strcmp(str1, str2) == 0;
}

void array_init(DynamicArray* array, size_t initial_capacity) {
    array->data = safe_malloc(initial_capacity * sizeof(void*));
    array->size = 0;
    array->capacity = initial_capacity;
}

void array_push(DynamicArray* array, void* item) {
    if (array->size >= array->capacity) {
        array->capacity *= 2;
        array->data = safe_realloc(array->data, array->capacity * sizeof(void*));
    }
    array->data[array->size++] = item;
}

void* array_get(const DynamicArray* array, size_t index) {
    if (index >= array->size) return NULL;
    return array->data[index];
}

void array_set(DynamicArray* array, size_t index, void* item) {
    if (index < array->size) {
        array->data[index] = item;
    }
}

void array_free(DynamicArray* array) {
    safe_free(array->data);
    array->data = NULL;
    array->size = 0;
    array->capacity = 0;
}

static size_t hash_function(const char* key, size_t capacity) {
    size_t hash = 5381;
    int c;
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % capacity;
}

HashTable* hashtable_create(size_t initial_capacity) {
    HashTable* table = safe_malloc(sizeof(HashTable));
    table->buckets = safe_malloc(initial_capacity * sizeof(HashTableEntry*));
    for (size_t i = 0; i < initial_capacity; i++) {
        table->buckets[i] = NULL;
    }
    table->size = 0;
    table->capacity = initial_capacity;
    return table;
}

void hashtable_destroy(HashTable* table) {
    if (!table) return;
    
    for (size_t i = 0; i < table->capacity; i++) {
        HashTableEntry* entry = table->buckets[i];
        while (entry) {
            HashTableEntry* next = entry->next;
            safe_free(entry->key);
            safe_free(entry);
            entry = next;
        }
    }
    safe_free(table->buckets);
    safe_free(table);
}

void* hashtable_get(HashTable* table, const char* key) {
    if (!table || !key) return NULL;
    
    size_t hash = hash_function(key, table->capacity);
    HashTableEntry* entry = table->buckets[hash];
    
    while (entry) {
        if (string_equal(entry->key, key)) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

void hashtable_put(HashTable* table, const char* key, void* value) {
    if (!table || !key) return;
    
    size_t hash = hash_function(key, table->capacity);
    HashTableEntry* entry = table->buckets[hash];
    
    while (entry) {
        if (string_equal(entry->key, key)) {
            entry->value = value;
            return;
        }
        entry = entry->next;
    }
    
    HashTableEntry* new_entry = safe_malloc(sizeof(HashTableEntry));
    new_entry->key = string_copy(key);
    new_entry->value = value;
    new_entry->next = table->buckets[hash];
    table->buckets[hash] = new_entry;
    table->size++;
}

bool hashtable_contains(HashTable* table, const char* key) {
    return hashtable_get(table, key) != NULL;
}

void hashtable_remove(HashTable* table, const char* key) {
    if (!table || !key) return;
    
    size_t hash = hash_function(key, table->capacity);
    HashTableEntry* entry = table->buckets[hash];
    HashTableEntry* prev = NULL;
    
    while (entry) {
        if (string_equal(entry->key, key)) {
            if (prev) {
                prev->next = entry->next;
            } else {
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