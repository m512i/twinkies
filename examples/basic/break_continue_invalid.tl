func main() -> int {
    // Invalid: break outside of loop
    break;
    
    // Invalid: continue outside of loop
    continue;
    
    // Valid: break/continue inside a loop
    let i: int = 0;
    while (i < 3) {
        if (i == 1) {
            break;
        }
        if (i == 2) {
            continue;
        }
        i = i + 1;
    }
    return 0;
} 