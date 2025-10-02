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
    
    return 0;
} 