// This is a comment at the top
func factorial(n: int) -> int {
    // Check if n is 0 or 1
    if (n <= 1) {
        return 1; // Base case
    }
    // Recursive case: n * factorial(n-1)
    return n * factorial(n - 1);
}

func main() -> int {
    // Calculate factorial of 5
    let result: int = factorial(5);
    print(result); // Print the result
    return 0; // Return success
} 