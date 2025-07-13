#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
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
    return (int64_t)strcmp(a, b);
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


int main(void) {
    char* message;
    char* greeting;
    int64_t number;
    char* a;
    char* b;
    char* c;
    char* d;
    int64_t len;
    char* sub;
    int64_t cmp;
    char* first_char;
    char* temp_0;
    char* temp_1;
    char* temp_2;
    int64_t temp_3;
    char* temp_4;
    int64_t temp_5;
    char* temp_6;

    message = "Hello, World!";
    printf("%s\n", message);
    greeting = "Welcome to Twink Language";
    printf("%s\n", greeting);
    number = 42;
    printf("%lld\n", number);
    a = "foo ";
    b = "bar ";
    temp_0 = __tl_concat(a, b);
    c = temp_0;
    printf("%s\n", c);
    temp_1 = __tl_concat(a, greeting);
    d = temp_1;
    printf("%s\n", d);
    temp_2 = __tl_concat("re", "tard");
    printf("%s\n", temp_2);
    temp_3 = __tl_strlen(message);
    len = temp_3;
    printf("%lld\n", len);
    temp_4 = __tl_substr(message, 0, 5);
    sub = temp_4;
    printf("%s\n", sub);
    temp_5 = __tl_strcmp("hello", "world");
    cmp = temp_5;
    printf("%lld\n", cmp);
    temp_6 = __tl_char_at(message, 0);
    first_char = temp_6;
    printf("%s\n", first_char);
    return 0;
}

