func test_basic() -> int {
    let x: int = 5;
    let y: int = 3;
    let result: int = x + y;
    return result;
}

func main() -> int {
    let answer: int = test_basic();
    print(answer);
    return 0;
} 