// ARRAY OUT OF BOUNDS TEST **WORKS CODEGEN C WILL WORK ON CODEGENASM LATER**
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
    
    // Test bounds checking (this should cause an error at runtime)
    print(arr[10]);
    
    return 0;
} 