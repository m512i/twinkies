func main() -> int {
    let i: int = 0;
    
    // Test break statement
    while (i < 10) {
        if (i == 5) {
            break;
        }
        print(i);
        i = i + 1;
    }
    
    print("After break loop");
    
    // Test continue statement
    let j: int = 0;
    while (j < 10) {
        j = j + 1;
        if (j == 5) {
            continue;
        }
        print(j);
    }
    
    print("After continue loop");
    
    return 0;
} 