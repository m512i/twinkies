// Math module implementation
// Functions from examples/demo.tl

// Function to calculate factorial (recursive)
func factorial(n: int) -> int {
    // Base case: factorial of 0 or 1 is 1
    if (n <= 1) {
        return 1;
    }
    // Recursive case: n! = n * (n-1)!
    return n * factorial(n - 1);
}

// Function to calculate the greatest common divisor using Euclidean algorithm
func gcd(a: int, b: int) -> int {
    // Base case: if b is 0, a is the GCD
    if (b == 0) {
        return a;
    }
    // Recursive case: GCD(a, b) = GCD(b, a % b)
    return gcd(b, a % b);
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