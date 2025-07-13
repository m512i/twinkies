func main() -> int {
    // Test 1: Simple break in while loop
    print("Test 1: Simple break");
    let i: int = 0;
    while (i < 5) {
        print(i);
        if (i == 3) {
            print("Breaking at i=3");
            break; // if continue; will inf loop
        }
        print(i);
        i = i + 1;
    }
    print("After break loop");
    
    // Test 2: Simple continue in while loop
    print("Test 2: Simple continue");
    let j: int = 0;
    while (j < 5) {
        if (j == 2) {
            print("Skipping j=2 with continue");
            j = j + 1;
            continue;
        }
        print(j);
        j = j + 1;
    }
    print("After continue loop");
    
    // Test 3: Break and continue in the same loop
    print("Test 3: Break and continue together");
    let k: int = 0;
    while (k < 10) {
        if (k == 1) {
            print("Continuing at k=1");
            k = k + 1;
            continue;
        }
        if (k == 5) {
            print("Breaking at k=5");
            break;
        }
        print(k);
        k = k + 1;
    }
    print("After mixed loop");
    
    // Test 4: Break in if statement inside loop
    print("Test 4: Break in nested if");
    let m: int = 0;
    while (m < 8) {
        if (m % 2 == 0) {
            if (m == 6) {
                print("Breaking at even number m=6");
                break;
            }
            print("Even number", m);
        } else {
            print("Odd number", m);
        }
        m = m + 1;
    }
    print("After nested break loop");
    
    // Test 5: Continue with different conditions
    print("Test 5: Multiple continue conditions");
    let n: int = 0;
    while (n < 7) {
        if (n == 1 || n == 3) {
            print("Skipping n", n);
            n = n + 1;
            continue;
        }
        if (n == 6) {
            print("Breaking at n=6");
            break;
        }
        print("Processing n", n);
        n = n + 1;
    }
    print("After multiple conditions loop");
    
    return 0;
} 