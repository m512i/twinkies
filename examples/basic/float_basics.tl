// Basic Float and Double Demo
// Simple examples of floating point usage

func main() -> int {
    // Declare float and double variables
    let my_float: float = 3.14;
    let my_double: double = 2.718;
    let my_int: int = 5;
    
    // Print the values
    print(my_float);
    print(my_double);
    print(my_int);
    
    // Basic arithmetic with floats
    let sum: float = my_float + 1.0;
    let product: float = my_float * 2.0;
    let quotient: float = my_float / 2.0;
    
    print(sum);      // Should print 4.14
    print(product);  // Should print 6.28
    print(quotient); // Should print 1.57
    
    // Mixed arithmetic (float + int)
    let mixed: double = my_float + my_int;
    print(mixed);    // Should print 8.14
    
    // Double precision arithmetic
    let double_sum: double = my_double + 1.0;
    let double_product: double = my_double * 3.0;
    
    print(double_sum);     // Should print 3.718
    print(double_product); // Should print 8.154
    
    // Float literals
    print(1.5);
    print(2.0 + 3.0);
    print(10.0 - 3.0);
    print(4.0 * 2.5);
    print(15.0 / 3.0);
    
    // Comparisons
    if (my_float > 3.0) {
        print(1);  // true
    } else {
        print(0);
    }
    
    if (my_double == 2.718) {
        print(1);  // true
    } else {
        print(0);
    }
    
    return 0;
} 