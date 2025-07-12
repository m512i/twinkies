func main() -> int {
    let message: string = "Hello, World!";
    print(message);
    
    let greeting: string = "Welcome to Twink Language";
    print(greeting);
    
    let number: int = 42;
    print(number);

    // String concatenation with +
    let a: string = "foo ";
    let b: string = "bar ";
    let c: string = a + b;
    print(c);

    // String concatenation with concat()
    let d: string = concat(a, greeting);
    print(d);
    print("re" + "tard");

    // String length
    let len: int = strlen(message);
    print(len);

    // String substring
    let sub: string = substr(message, 0, 5);
    print(sub);

    // String comparison
    let cmp: int = strcmp("hello", "world");
    print(cmp);

    // String indexing
    let first_char: string = message[0];
    print(first_char);
    
    // Arithmetic operations
    let x: int = 10;
    let y: int = 5;
    let sum: int = x + y;
    let diff: int = x - y;
    let product: int = x * y;
    let quotient: int = x / y;
    let remainder: int = x % y;
    
    print(sum);
    print(diff);
    print(product);
    print(quotient);
    print(remainder);
    
    // Boolean operations
    let is_true: bool = true;
    let is_false: bool = false;
    let and_result: bool = is_true && is_false;
    let or_result: bool = is_true || is_false;
    let not_result: bool = !is_false;
    
    print(and_result);
    print(or_result);
    print(not_result);
    
    // Comparison operations
    let eq_test: bool = x == y;
    let ne_test: bool = x != y;
    let lt_test: bool = x < y;
    let gt_test: bool = x > y;
    let le_test: bool = x <= y;
    let ge_test: bool = x >= y;
    
    print(eq_test);
    print(ne_test);
    print(lt_test);
    print(gt_test);
    print(le_test);
    print(ge_test);
    
    // Array operations
    let arr: int[5] = 0;
    arr[0] = 10;
    arr[1] = 20;
    arr[2] = 30;
    arr[3] = 40;
    arr[4] = 50;
    
    print(arr[0]);
    print(arr[1]);
    print(arr[2]);
    print(arr[3]);
    print(arr[4]);
    
    // Control flow - if statements
    if (x > y) {
        print(100);
    } else {
        print(200);
    }
    
    if (x == y) {
        print(300);
    } else {
        print(400);
    }
    
    // Control flow - while loops
    let counter: int = 0;
    while (counter < 3) {
        print(counter);
        counter = counter + 1;
    }
    
    // Nested expressions
    let complex_expr: int = (x + y) * 2 - (x / y);
    print(complex_expr);
    
    // More string operations
    let str1: string = "Hello";
    let str2: string = "World";
    let combined: string = str1 + " " + str2;
    print(combined);
    
    let str_len: int = strlen(combined);
    print(str_len);
    
    let sub_str: string = substr(combined, 0, 5);
    print(sub_str);
    
    let char_at: string = combined[6];
    print(char_at);
    
    // Function calls with multiple arguments
    let test_str: string = "Testing";
    let test_len: int = strlen(test_str);
    let test_sub: string = substr(test_str, 0, 4);
    let test_cmp: int = strcmp(test_str, "Testing");
    
    print(test_len);
    print(test_sub);
    print(test_cmp);
    
    // Negative numbers and unary operations
    let neg_x: int = -x;
    let abs_diff: int = x - y;
    if (abs_diff < 0) {
        abs_diff = -abs_diff;
    }
    
    print(neg_x);
    print(abs_diff);
    
    // Boolean expressions with arithmetic
    let bool_arith: bool = (x + y) > 10;
    let bool_arith2: bool = (x * y) == 50;
    
    print(bool_arith);
    print(bool_arith2);
    
    // String array test
    let str_arr: string[3] = "";
    str_arr[0] = "First";
    str_arr[1] = "Second";
    str_arr[2] = "Third";
    
    print(str_arr[0]);
    print(str_arr[1]);
    print(str_arr[2]);
    
    // Complex boolean logic
    let complex_bool: bool = (x > 5) && (y < 10) || (x == 10);
    print(complex_bool);
    
    // Nested if-else
    if (x > 15) {
        print(1000);
    } else if (x > 10) {
        print(2000);
    } else if (x > 5) {
        print(3000);
    } else {
        print(4000);
    }
    
    // More string concatenation tests
    let part1: string = "Hello";
    let part2: string = " ";
    let part3: string = "Twink";
    let part4: string = "!";
    
    let final_str: string = part1 + part2 + part3 + part4;
    print(final_str);
    
    // Array bounds testing (should work within bounds)
    let safe_index: int = 2;
    if (safe_index >= 0 && safe_index < 5) {
        print(arr[safe_index]);
    }
    
    // String comparison tests
    let str_a: string = "apple";
    let str_b: string = "banana";
    let str_c: string = "apple";
    
    let cmp1: int = strcmp(str_a, str_b);
    let cmp2: int = strcmp(str_b, str_a);
    let cmp3: int = strcmp(str_a, str_c);
    
    print(cmp1);
    print(cmp2);
    print(cmp3);
    
    return 0;
} 