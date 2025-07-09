func main() -> int {
    // Test different array sizes
    let small_arr: int[3];
    let medium_arr: int[5];
    let large_arr: int[10];
    
    let i: int = 0;
    
    // Initialize small array (size 3)
    while (i < 3) {
        small_arr[i] = i * 10;
        i = i + 1;
    }
    
    // Initialize medium array (size 5)
    i = 0;
    while (i < 5) {
        medium_arr[i] = i * 2;
        i = i + 1;
    }
    
    // Initialize large array (size 10)
    i = 0;
    while (i < 10) {
        large_arr[i] = i * 3;
        i = i + 1;
    }
    
    // Print small array (should print: 0, 10, 20)
    print(1000); // Marker
    i = 0;
    while (i < 3) {
        print(small_arr[i]);
        i = i + 1;
    }
    
    // Print medium array (should print: 0, 2, 4, 6, 8)
    print(2000); // Marker
    i = 0;
    while (i < 5) {
        print(medium_arr[i]);
        i = i + 1;
    }
    
    // Print large array (should print: 0, 3, 6, 9, 12, 15, 18, 21, 24, 27)
    print(3000); // Marker
    i = 0;
    while (i < 10) {
        print(large_arr[i]);
        i = i + 1;
    }
    
    // Test bounds checking - these should all work (in bounds)
    print(4000); // Marker
    print(small_arr[2]);  // Should print 20 (last element of small_arr)
    print(medium_arr[4]); // Should print 8 (last element of medium_arr)
    print(large_arr[9]);  // Should print 27 (last element of large_arr)
    
    // Test out-of-bounds access - this should cause runtime error
    print(5000); // Marker
    print(small_arr[3]);  // Out of bounds! Should cause "Array index out of bounds" error
    
    return 0;
} 