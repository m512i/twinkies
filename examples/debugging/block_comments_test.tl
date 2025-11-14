// Test file for block comments

/*
yes after 25 million years i have added a block comment
*/

/* This is a single-line block comment */

func test_single_line_block() -> int {
    /* Comment before code */
    let x: int = 42; /* Comment after code */
    return x;
}

func test_multi_line_block() -> int {
    /* This is a multi-line
       block comment that
       spans multiple lines */
    let result: int = 10;
    return result;
}

func test_nested_code() -> int {
    /* Outer comment */
    let a: int = 1; /* Comment in middle */ let b: int = 2;
    /* Another comment */
    return a + b;
}

func main() -> int {
    /* Test all comment types */
    print(test_single_line_block()); // Single-line comment
    print(test_multi_line_block());  /* Block comment */
    
    /* Multi-line comment
       with multiple
       lines of text */
    print(test_nested_code());
    
    return 0;
}