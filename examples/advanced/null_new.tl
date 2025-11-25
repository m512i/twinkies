func main() -> int {
    // Demonstrating null used directly in operations (not just in variables)
    
    print("=== Null in Direct Operations ===");
    
    // Arithmetic operations with null literals
    let result1: int = 10 + null;
    let result2: int = null * 5;
    let result3: int = 100 - null;
    let result4: int = null + null;
    
    print("10 + null =");
    print(result1);
    print("null * 5 =");
    print(result2);
    print("100 - null =");
    print(result3);
    print("null + null =");
    print(result4);
    
    // Comparisons with null literals
    let comp1: bool = 5 > null;
    let comp2: bool = null < 10;
    let comp3: bool = null == null;
    let comp4: bool = null != 0;
    
    print("5 > null =");
    print(comp1);
    print("null < 10 =");
    print(comp2);
    print("null == null =");
    print(comp3);
    print("null != 0 =");
    print(comp4);
    
    // Boolean operations with null
    let bool1: bool = true && null;
    let bool2: bool = null || false;
    let bool3: bool = !null;
    
    print("true && null =");
    print(bool1);
    print("null || false =");
    print(bool2);
    print("!null =");
    print(bool3);
    
    // String operations with null
    let str1: string = "Hello" + null;
    let str2: string = null + "World";
    let str3: string = null + null;
    
    print("'Hello' + null =");
    print(str1);
    print("null + 'World' =");
    print(str2);
    print("null + null =");
    print(str3);
    
    // Complex expressions with null
    let complex1: int = (10 + null) * 2;
    let complex2: int = null + (5 * null);
    let complex3: bool = (10 > null) && (null == null);
    
    print("(10 + null) * 2 =");
    print(complex1);
    print("null + (5 * null) =");
    print(complex2);
    print("(10 > null) && (null == null) =");
    print(complex3);
    
    // Unary operations with null
    let unary1: int = -null;
    let unary2: bool = !null;
    
    print("-null =");
    print(unary1);
    print("!null =");
    print(unary2);
    
    // Null in conditionals
    if (null) {
        print("null is truthy");
    } else {
        print("null is falsy");
    }
    
    if (10 + null > 5) {
        print("10 + null > 5 is true");
    } else {
        print("10 + null > 5 is false");
    }
    
    // Float/double operations with null
    let float_result: float = 3.14 + null;
    let double_result: double = null * 2.5;
    
    print("3.14 + null =");
    print(float_result);
    print("null * 2.5 =");
    print(double_result);
    
    return 0;
}

