// Void function with no return statement
func print_hello() -> void {
    print("Hello from void function!");
}

// Void function with explicit return
func print_goodbye() -> void {
    print("Goodbye!");
    return;  // Explicit return without value
}

// Void function with early return
func conditional_print(flag: bool) -> void {
    if (!flag) {
        return;  // Early return
    }
    print("Flag is true!");
}

// Non-void function for comparison
func get_value() -> int {
    return 42;
}

// Function that calls void functions
func test_void_functions() -> int {
    print("=== Testing Void Functions ===");
    
    // Call void functions
    print_hello();
    print_goodbye();
    conditional_print(true);
    conditional_print(false);
    
    // Call non-void function
    let value: int = get_value();
    print("Non-void function returned:", value);
    
    print("=== Test Complete ===");
    return 0;
}

func main() -> int {
    test_void_functions();
    return 0;
}

