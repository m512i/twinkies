// TEST FOR BOOLEAN LITERALS **WORKING FOR CODEGEN.C NOT CODEGENASM.C (WILL FIX LATER)

func print_bool(b: bool) -> int {
    if (b) {
        print(1);
    } else {
        print(0);
    }
    return 0;
}

func bool_and(a: bool, b: bool) -> bool {
    return a && b;
}

func main() -> int {
    let t: bool = true;
    let f: bool = false;
    let n: int = 42;
    let b1: bool = bool_and(true, false); // OK
    let b2: bool = bool_and(t, f);        // OK
    print_bool(true);                     // OK
    print_bool(false);                    // OK
    print_bool(b1);                       // OK
    // let err1: bool = 1;               // ERROR: int assigned to bool
    // let err2: int = true;             // ERROR: bool assigned to int
    // let err3: bool = bool_and(1, 0);  // ERROR: int arguments to bool_and
    // print_bool(42);                   // ERROR: int passed to bool param
    return 0;
} 