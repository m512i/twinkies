// TEST FOR FUNCTION SIGNATURE CHECKING **WORKING**

func add(x: int) -> int {
    return x + 1;
}

func add(x: int, y: int) -> int {
    return x + y;
}

func add(flag: bool) -> int {
    if (flag) {
        return 42;
    } else {
        return 0;
    }
}

func greet() -> int {
    print(42);
    return 0;
}

func main() -> int {
    let a: int = add(5);         
    let b: int = add(2, 3);     
    let c: int = add(true);     
    let d: int = add(false);    
    let e: int = add();         
    let f: int = add(1, true); 
    let g: int = add(1, 2, 3);  
    greet();                    
    return 0;
}