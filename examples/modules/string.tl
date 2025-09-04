// String module implementation
// Functions from examples/string_test.tl

// Get the length of a string
func string_length(str: string) -> int {
    // Simple implementation - count characters until null terminator
    let len: int = 0;
    let i: int = 0;
    while (str[i] != "\0") {
        len = len + 1;
        i = i + 1;
    }
    return len;
}

// Compare two strings
func string_compare(str1: string, str2: string) -> int {
    let i: int = 0;
    while (str1[i] != "\0" && str2[i] != "\0") {
        if (str1[i] != str2[i]) {
            if (str1[i] < str2[i]) {
                return -1;
            } else {
                return 1;
            }
        }
        i = i + 1;
    }
    
    // Check if both strings ended at the same time
    if (str1[i] == "\0" && str2[i] == "\0") {
        return 0; // Strings are equal
    } else if (str1[i] == "\0") {
        return -1; // str1 is shorter
    } else {
        return 1; // str2 is shorter
    }
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