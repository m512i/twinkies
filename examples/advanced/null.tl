func main() -> int {
    // Test 1: Basic null literal assignments
    let null_int: int = null;
    let null_string: string = null;
    let null_bool: bool = null;
    
    print("=== Basic Null Tests ===");
    print(null_int);
    print(null_string);
    print(null_bool);
    
    // Test 2: Null comparisons
    print("=== Null Comparison Tests ===");
    if (null_int == null) {
        print("null_int equals null");
    } else {
        print("null_int does not equal null");
    }
    
    if (null_string == null) {
        print("null_string equals null");
    } else {
        print("null_string does not equal null");
    }
    
    if (null_bool == null) {
        print("null_bool equals null");
    } else {
        print("null_bool does not equal null");
    }
    
    // Test 3: Null in arithmetic expressions
    print("=== Null in Arithmetic ===");
    let x: int = 10;
    let result1: int = x + null_int;
    let result2: int = null_int + x;
    let result3: int = null_int + null_int;
    
    print(result1);
    print(result2);
    print(result3);
    
    // Test 4: Null in boolean expressions
    print("=== Null in Boolean Logic ===");
    let bool1: bool = true;
    let bool2: bool = false;
    
    let and_result1: bool = bool1 && null_bool;
    let and_result2: bool = null_bool && bool1;
    let or_result1: bool = bool1 || null_bool;
    let or_result2: bool = null_bool || bool1;
    
    print(and_result1);
    print(and_result2);
    print(or_result1);
    print(or_result2);
    
    // Test 5: Null in comparison expressions
    print("=== Null in Comparisons ===");
    let comp1: bool = x > null_int;
    let comp2: bool = null_int < x;
    let comp3: bool = null_int == null_int;
    let comp4: bool = null_int != null_int;
    
    print(comp1);
    print(comp2);
    print(comp3);
    print(comp4);
    
    // Test 6: Null in string operations
    print("=== Null in String Operations ===");
    let str1: string = "Hello";
    let str2: string = "World";
    
    let concat1: string = str1 + null_string;
    let concat2: string = null_string + str1;
    let concat3: string = null_string + null_string;
    
    print(concat1);
    print(concat2);
    print(concat3);
    
    // Test 7: Null in conditional expressions
    print("=== Null in Conditionals ===");
    if (null_bool) {
        print("null_bool is true");
    } else {
        print("null_bool is false");
    }
    
    if (null_int > 0) {
        print("null_int is positive");
    } else {
        print("null_int is not positive");
    }
    
    // Test 8: Null reassignment
    print("=== Null Reassignment ===");
    let var1: int = 42;
    print(var1);
    
    var1 = null;
    print(var1);
    
    var1 = 100;
    print(var1);
    
    // Test 9: Null in complex expressions
    print("=== Null in Complex Expressions ===");
    let complex1: int = (x + null_int) * 2;
    let complex2: bool = (x > null_int) && (null_bool || true);
    let complex3: string = str1 + (null_string + "test");
    
    print(complex1);
    print(complex2);
    print(complex3);
    
    // Test 10: Null edge cases
    print("=== Null Edge Cases ===");
    let edge1: int = -null_int;
    let edge2: bool = !null_bool;
    let edge3: int = null_int % 5;
    let edge4: int = null_int / 2;
    
    print(edge1);
    print(edge2);
    print(edge3);
    print(edge4);
    
    // Test 11: Null in nested expressions
    print("=== Null in Nested Expressions ===");
    let nested1: int = ((x + null_int) * 2) + (null_int + 5);
    let nested2: bool = ((x > null_int) && (null_bool || true)) || false;
    let nested3: string = (str1 + null_string) + ("test" + null_string);
    
    print(nested1);
    print(nested2);
    print(nested3);
    
    // Test 12: Null with different data types
    print("=== Null with Different Types ===");
    let float_null: float = null;
    let double_null: double = null;
    
    print(float_null);
    print(double_null);
    
    let float_result: float = float_null + 3.14;
    let double_result: double = double_null * 2.0;
    
    print(float_result);
    print(double_result);
    
    return 0;
} 