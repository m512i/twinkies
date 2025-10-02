// Main program demonstrating module system with implemented features (ALSO THE DOUBLE I IN INCLUDE WILL BE FIXED LOL)
#iinclude "math.tlh"
#iinclude "string.tlh"

func main() -> int {
    let a: int = 10;
    let b: int = 5;
    let sum: int = a + b;
    let diff: int = a - b;
    let product: int = a * b;
    let quotient: int = a / b;
    let remainder: int = a % b;
    
    print("Basic arithmetic test:");
    print("a = ");
    print(a);
    print("b = ");
    print(b);
    print("sum = ");
    print(sum);
    print("diff = ");
    print(diff);
    print("product = ");
    print(product);
    print("quotient = ");
    print(quotient);
    print("remainder = ");
    print(remainder);
    
    let fact_result: int = factorial(5);
    print("factorial(5) = ");
    print(fact_result);
    
    let str1: string = "Hello";
    let str2: string = "World";
    print("String test:");
    print("str1 = ");
    print(str1);
    print("str2 = ");
    print(str2);
    let combined: string = string_concat(str1, " ");
    combined = string_concat(combined, str2);
    print("combined = ");
    print(combined);
    let len: int = string_length(str1);
    print("string_length(str1) = ");
    print(len);
    let cmp: int = string_compare(str1, str2);
    print("string_compare(str1, str2) = ");
    print(cmp);
    
    let arr: int[5];
    let i: int = 0;
    while (i < 5) {
        arr[i] = i * 10;
        i = i + 1;
    }
    print("Array test:");
    i = 0;
    while (i < 5) {
        print("arr[");
        print(i);
        print("] = ");
        print(arr[i]);
        i = i + 1;
    }
    
    let my_float: float = 3.14;
    let my_double: double = 2.718;
    print("Float test:");
    print("my_float = ");
    print(my_float);
    print("my_double = ");
    print(my_double);
    let float_sum: float = my_float + 1.0;
    let float_product: float = my_float * 2.0;
    print("float_sum = ");
    print(float_sum);
    print("float_product = ");
    print(float_product);
    
    print("Comparison test:");
    if (a > b) {
        print("a > b is true");
    } else {
        print("a > b is false");
    }
    if (a == 10) {
        print("a == 10 is true");
    } else {
        print("a == 10 is false");
    }
    
    let gcd_result: int = gcd(48, 18);
    print("gcd(48, 18) = ");
    print(gcd_result);
    
    let fib_result: int = fibonacci(6);
    print("fibonacci(6) = ");
    print(fib_result);
    
    let test_prime: int = 17;
    if (is_prime(test_prime)) {
        print("17 is prime");
    } else {
        print("17 is not prime");
    }
    
    let test_non_prime: int = 15;
    if (is_prime(test_non_prime)) {
        print("15 is prime");
    } else {
        print("15 is not prime");
    }
    
    return 0;
} 