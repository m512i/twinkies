func main() -> int {
    // Unused variable warning
    let unused_var: int = 42;
    
    // Type conversion warning
    let bool_var: bool = 1;  // Implicit int to bool conversion
    
    // Performance warning - division by 1
    let result: int = 10 / 1;
    
    // Performance warning - division by 0
    let zero_div: int = 10 / 0;
    
    // Unreachable code warning
    return 0;
    let unreachable: int = 5;  // This will never be executed
    
    // Used variable (no warning)
    let used_var: int = 100;
    print(used_var);
    
    return 0;
} 