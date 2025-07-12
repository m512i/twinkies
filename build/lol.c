#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>


int main(void) {
    char* message;
    char* greeting;
    int64_t number;
    message = "Hello, World!";
    printf("%s\n", message);
    greeting = "Welcome to Twink Language";
    printf("%s\n", greeting);
    number = 42;
    printf("%lld\n", number);
    printf("%s\n", "lol");
    return 0;
}

