// FLOATING POINT TEST

func main() -> int {
    let x: float = 3.14;
    let y: double = 2.718;
    let z: int = 5;
    
    // Test floating point arithmetic
    let result: double = x + y;
    print(result);
    
    // Test mixed arithmetic (int + float)
    let mixed: double = z + x;
    print(mixed);
    
    // Test floating point literals
    print(1.5);
    print(2.0 + 3.0);
    
    return 0;
} 