#ifndef RUNTIME_H
#define RUNTIME_H

#include <stdint.h>

char* __tl_concat(const char* a, const char* b);
int64_t __tl_strlen(const char* str);
char* __tl_substr(const char* str, int64_t start, int64_t len);
int64_t __tl_strcmp(const char* a, const char* b);
char* __tl_char_at(const char* str, int64_t index);

#endif