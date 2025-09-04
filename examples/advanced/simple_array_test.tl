func main() -> int {
    let arr: int[5];
    let i: int = 0;
    
    // Initialize array
    while (i < 5) {
        arr[i] = i * 2;
        i = i + 1;
    }
    
    // Print array elements
    i = 0;
    while (i < 5) {
        print(arr[i]);
        i = i + 1;
    }
    
    // Test bounds checking - this should work (in bounds)
    print(1000); // Marker
    print(arr[4]);  // Should print 8 (last element)
    
    // Test out-of-bounds access - this should cause runtime error
    print(2000); // Marker
    print(arr[5]);  // Out of bounds! Should cause "Array index out of bounds" error
    
    return 0;
} 