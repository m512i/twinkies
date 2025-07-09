// ========================================
// Tiny Language Compiler Demo
// This file demonstrates all features of the TL compiler
// ========================================

// Function to calculate the greatest common divisor using Euclidean algorithm
func gcd(a: int, b: int) -> int {
    // Base case: if b is 0, a is the GCD
    if (b == 0) {
        return a;
    }
    // Recursive case: GCD(a, b) = GCD(b, a % b)
    return gcd(b, a % b);
}

// Function to check if a number is prime
func is_prime(n: int) -> int {
    // Numbers less than 2 are not prime
    if (n < 2) {
        return 0; // false
    }
    
    // Check divisibility from 2 to sqrt(n)
    let i: int = 2;
    while (i * i <= n) {
        // If n is divisible by i, it's not prime
        if (n % i == 0) {
            return 0; // false
        }
        i = i + 1;
    }
    return 1; // true
}

// Function to calculate factorial (recursive)
func factorial(n: int) -> int {
    // Base case: factorial of 0 or 1 is 1
    if (n <= 1) {
        return 1;
    }
    // Recursive case: n! = n * (n-1)!
    return n * factorial(n - 1);
}

// Function to calculate fibonacci numbers
func fibonacci(n: int) -> int {
    // Base cases: fib(0) = 0, fib(1) = 1
    if (n <= 1) {
        return n;
    }
    // Recursive case: fib(n) = fib(n-1) + fib(n-2)
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// Function to demonstrate arithmetic operations
func arithmetic_demo() -> int {
    let a: int = 15;
    let b: int = 4;
    
    // Basic arithmetic operations
    let sum: int = a + b;        // 15 + 4 = 19
    let diff: int = a - b;       // 15 - 4 = 11
    let product: int = a * b;    // 15 * 4 = 60
    let quotient: int = a / b;   // 15 / 4 = 3 (integer division)
    let remainder: int = a % b;  // 15 % 4 = 3
    
    // Print results
    print(sum);      // Should print 19
    print(diff);     // Should print 11
    print(product);  // Should print 60
    print(quotient); // Should print 3
    print(remainder); // Should print 3
    
    return 0;
}

// Function to demonstrate comparison operators
func comparison_demo() -> int {
    let x: int = 10;
    let y: int = 5;
    
    // Test various comparisons
    if (x > y) {
        print(1); // true
    } else {
        print(0); // false
    }
    
    if (x >= y) {
        print(1); // true
    } else {
        print(0); // false
    }
    
    if (x == y) {
        print(1); // false
    } else {
        print(0); // true
    }
    
    if (x != y) {
        print(1); // true
    } else {
        print(0); // false
    }
    
    return 0;
}

// Function to demonstrate nested function calls
func nested_calls() -> int {
    // Calculate factorial of 5
    let fact_5: int = factorial(5);
    
    // Calculate GCD of 48 and 18
    let gcd_result: int = gcd(48, 18);
    
    // Calculate 6th fibonacci number
    let fib_6: int = fibonacci(6);
    
    // Print results
    print(fact_5);   // Should print 120
    print(gcd_result); // Should print 6
    print(fib_6);    // Should print 8
    
    return 0;
}

// Main function - entry point of the program
func main() -> int {
    // Welcome message
    print(42); // Simple print to show the program is running
    
    // Run arithmetic demonstration
    arithmetic_demo();
    
    // Run comparison demonstration
    comparison_demo();
    
    // Run nested function calls demonstration
    nested_calls();
    
    // Test prime number checking
    let test_numbers: int = 17;
    if (is_prime(test_numbers)) {
        print(1); // 17 is prime
    } else {
        print(0);
    }
    
    // Test with a non-prime number
    let non_prime: int = 15;
    if (is_prime(non_prime)) {
        print(1);
    } else {
        print(0); // 15 is not prime
    }
    
    // Final success message
    print(0); // Return value indicating success
    
    return 0; // Program completed successfully
} 