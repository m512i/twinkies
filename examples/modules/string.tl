// String module implementation
// Functions from examples/string_test.tl

// Get the length of a string
func string_length(str: string) -> int {
    // Use the runtime function for string length
    return __tl_strlen(str);
}

// Compare two strings
func string_compare(str1: string, str2: string) -> int {
    // Use the runtime function for string comparison
    return __tl_strcmp(str1, str2);
}

// Extract a substring
func string_substr(str: string, start: int, length: int) -> string {
    // Use the runtime function for substring
    return __tl_substr(str, start, length);
}

// Concatenate two strings
func string_concat(str1: string, str2: string) -> string {
    // Use the runtime function for concatenation
    return __tl_concat(str1, str2);
} 