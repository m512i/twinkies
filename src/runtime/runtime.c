#include "runtime/runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* __tl_concat(const char* a, const char* b) {
    if (!a) a = "";
    if (!b) b = "";
    size_t len_a = strlen(a);
    size_t len_b = strlen(b);
    char* result = (char*)malloc(len_a + len_b + 1);
    if (!result) { fprintf(stderr, "Out of memory\n"); exit(1); }
    strcpy(result, a);
    strcat(result, b);
    return result;
}

int64_t __tl_strlen(const char* str) {
    if (!str) return 0;
    return (int64_t)strlen(str);
}

char* __tl_substr(const char* str, int64_t start, int64_t len) {
    if (!str) return strdup("");
    size_t str_len = strlen(str);
    if (start < 0 || start >= str_len || len < 0) {
        return strdup("");
    }
    if (start + len > str_len) {
        len = str_len - start;
    }
    char* result = (char*)malloc(len + 1);
    if (!result) { fprintf(stderr, "Out of memory\n"); exit(1); }
    strncpy(result, str + start, len);
    result[len] = '\0';
    return result;
}

int64_t __tl_strcmp(const char* a, const char* b) {
    if (!a && !b) return 0;
    if (!a) return -1;
    if (!b) return 1;
    // Unix-style implementation that returns actual character difference
    while (*a != '\0' && *a == *b) {
        a++;
        b++;
    }
    return (int64_t)((unsigned char)*a - (unsigned char)*b);
}

char* __tl_char_at(const char* str, int64_t index) {
    if (!str) return strdup("");
    size_t len = strlen(str);
    if (index < 0 || index >= len) {
        return strdup("");
    }
    char* result = (char*)malloc(2);
    if (!result) { fprintf(stderr, "Out of memory\n"); exit(1); }
    result[0] = str[index];
    result[1] = '\0';
    return result;
}

