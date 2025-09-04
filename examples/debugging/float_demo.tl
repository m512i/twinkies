// ========================================
// Floating Point Demo for TL Compiler
// Demonstrates float and double usage
// ========================================

// Function that takes float parameters and returns double
func calculate_area(radius: float, pi: float) -> double {
    let area: double = pi * radius * radius;
    return area;
}

// Function demonstrating mixed arithmetic
func mixed_arithmetic(x: float, y: int) -> double {
    let result: double = x + y;  // float + int = double
    return result;
}

// Function with double precision calculations
func precision_demo() -> double {
    let a: double = 3.14159265359;
    let b: double = 2.71828182846;
    let c: double = 1.41421356237;
    
    let result: double = a * b + c;
    return result;
}

// Function demonstrating type conversions
func type_conversions() -> int {
    let float_val: float = 3.14;
    let double_val: double = 2.718;
    let int_val: int = 5;
    
    // Convert float to int (truncation)
    let float_to_int: int = float_val;
    
    // Convert int to float
    let int_to_float: float = int_val;
    
    // Convert double to float (precision loss)
    let double_to_float: float = double_val;
    
    print(float_to_int);    // Should print 3
    print(int_to_float);    // Should print 5.0
    print(double_to_float); // Should print 2.718
    
    return 0;
}

// Function demonstrating floating point literals
func literal_demo() -> double {
    // Various floating point literals
    let small_float: float = 0.001;
    let large_float: float = 123456.789;
    let scientific: double = 1.23e-4;
    let negative: double = -42.5;
    
    print(small_float);
    print(large_float);
    print(scientific);
    print(negative);
    
    // Direct literal usage
    print(3.14159);
    print(2.0 + 3.0);
    print(10.5 - 2.3);
    print(4.2 * 2.1);
    print(15.0 / 3.0);
    
    return 0.0;
}

// Function demonstrating comparison operations
func comparison_demo() -> int {
    let a: float = 3.14;
    let b: double = 3.14;
    let c: float = 2.71;
    
    // Compare floats and doubles
    if (a == 3.14) {
        print(1);  // true
    } else {
        print(0);
    }
    
    if (b > a) {
        print(1);  // true (double > float)
    } else {
        print(0);
    }
    
    if (c < a) {
        print(1);  // true
    } else {
        print(0);
    }
    
    return 0;
}

// Main function demonstrating all features
func main() -> int {
    print(42);  // Welcome message
    
    // Test area calculation
    let radius: float = 5.0;
    let pi: float = 3.14159;
    let area: double = calculate_area(radius, pi);
    print(area);  // Should print ~78.54
    
    // Test mixed arithmetic
    let mixed_result: double = mixed_arithmetic(3.5, 2);
    print(mixed_result);  // Should print 5.5
    
    // Test precision demo
    let precision_result: double = precision_demo();
    print(precision_result);
    
    // Test type conversions
    type_conversions();
    
    // Test literal demo
    literal_demo();
    
    // Test comparisons
    comparison_demo();
    
    // Test array with floats
    let float_array: float[3];
    float_array[0] = 1.1;
    float_array[1] = 2.2;
    float_array[2] = 3.3;
    
    print(float_array[0]);
    print(float_array[1]);
    print(float_array[2]);
    
    // Test while loop with floats
    let counter: float = 0.0;
    while (counter < 3.0) {
        print(counter);
        counter = counter + 1.0;
    }
    
    return 0;
} 