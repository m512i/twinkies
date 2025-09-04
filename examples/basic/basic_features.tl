// Basic Features Demo - Showcases core .tl language features
// This example demonstrates variables, functions, control flow, and basic operations

// Function with parameters and return value
func add(a: int, b: int) -> int {
    return a + b;
}

// Function with multiple return statements
func absolute_value(x: int) -> int {
    if (x >= 0) {
        return x;
    } else {
        return -x;
    }
}

// Function that demonstrates loops and arrays
func array_demo() -> int {
    // Array declaration and initialization
    let numbers: int[5];
    let i: int = 0;
    
    // Initialize array with values
    while (i < 5) {
        numbers[i] = i * i;  // Square of index
        i = i + 1;
    }
    
    // Print array contents
    i = 0;
    while (i < 5) {
        print(numbers[i]);
        i = i + 1;
    }
    
    return 0;
}

// Function demonstrating string operations
func string_demo() -> int {
    let greeting: string = "Hello";
    let name: string = "World";
    
    // String concatenation
    let message: string = greeting + ", " + name + "!";
    print(message);
    
    // String functions
    let length: int = strlen(message);
    print(length);
    
    let substring: string = substr(message, 0, 5);
    print(substring);
    
    return 0;
}

// Function demonstrating boolean logic
func boolean_demo() -> int {
    let a: bool = true;
    let b: bool = false;
    
    // Boolean operations
    let and_result: bool = a && b;
    let or_result: bool = a || b;
    let not_result: bool = !b;
    
    print(and_result);  // false
    print(or_result);   // true
    print(not_result);  // true
    
    // Comparison operators
    let x: int = 10;
    let y: int = 5;
    
    if (x > y) {
        print(1);  // true
    } else {
        print(0);  // false
    }
    
    if (x == 10 && y < 10) {
        print(1);  // true
    } else {
        print(0);  // false
    }
    
    return 0;
}

// Function demonstrating nested control structures
func control_flow_demo() -> int {
    let i: int = 0;
    
    while (i < 10) {
        if (i == 0) {
            print(0);  // Print 0
        } else if (i % 2 == 0) {
            print(i);  // Print even numbers
        } else {
            // Skip odd numbers
            i = i + 1;
            continue;
        }
        
        if (i == 8) {
            break;  // Exit loop early
        }
        
        i = i + 1;
    }
    
    return 0;
}

// Main function that demonstrates all features
func main() -> int {
    print("=== Basic Features Demo ===");
    
    // Variable declarations and arithmetic
    let x: int = 15;
    let y: int = 7;
    let result: int = add(x, y);
    print(result);  // Should print 22
    
    // Function calls
    let abs_val: int = absolute_value(-42);
    print(abs_val);  // Should print 42
    
    // Array demonstration
    print("Array demo:");
    array_demo();
    
    // String demonstration
    print("String demo:");
    string_demo();
    
    // Boolean demonstration
    print("Boolean demo:");
    boolean_demo();
    
    // Control flow demonstration
    print("Control flow demo:");
    control_flow_demo();
    
    // Float operations
    let pi: float = 3.14159;
    let radius: float = 5.0;
    let area: float = pi * radius * radius;
    print(area);
    
    print("Demo completed successfully!");
    return 0;
}
