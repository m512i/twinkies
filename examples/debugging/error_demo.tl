func main() -> int {
    // Undefined variable error
    let x: int = undefined_var;
    
    // Type mismatch error
    let y: bool = 42;
    
    // Array initialization error
    let arr: int[5] = "hello";
    
    // Missing semicolon
    let z: int = 10
    
    // Array bounds error
    arr[10] = 5;
    
    // Invalid operation
    let result: bool = 5 + true;
    
    // Missing closing brace
    if (x > 0) {
        print(x);
    
    return 0;
} 