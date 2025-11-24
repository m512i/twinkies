func main() -> int {
    // Menu-driven program
    print("=== Menu System ===");
    print("1. Play guessing game");
    print("2. Check number properties");
    print("3. Simple calculator");
    print("4. Exit");
    print("Enter your choice: ");
    let choice: int = input();
    
    if (choice == 1) {
        // Guessing game
        print("\n=== Guessing Game ===");
        print("Think of a number between 1 and 100.");
        print("I'll try to guess it!");
        print("Enter 1 if my guess is too high");
        print("Enter 2 if my guess is too low");
        print("Enter 3 if I guessed correctly");
        print("Enter 0 to quit");
        
        let min: int = 1;
        let max: int = 100;
        let guess: int = 50;
        let feedback: int = 0;
        let attempts: int = 0;
        
        while (feedback != 3 && feedback != 0) {
            print("My guess is: ", guess);
            print("Your feedback: ");
            feedback = input();
            attempts = attempts + 1;
            
            if (feedback == 1) {
                max = guess - 1;
                guess = (min + max) / 2;
            } else if (feedback == 2) {
                min = guess + 1;
                guess = (min + max) / 2;
            } else if (feedback == 3) {
                print("I guessed it in ", attempts, " attempts!");
            } else if (feedback == 0) {
                print("Game quit.");
            } else {
                print("Invalid input! Use 1, 2, 3, or 0.");
            }
        }
    } else if (choice == 2) {
        // Number properties checker
        print("\n=== Number Properties ===");
        print("Enter a number: ");
        let num: int = input();
        
        print("Properties of ", num, ":\n");
        
        // Check if even/odd
        if (num % 2 == 0) {
            print("- Even number");
        } else {
            print("- Odd number");
        }
        
        // Check if positive/negative/zero
        if (num > 0) {
            print("- Positive");
        } else if (num < 0) {
            print("- Negative");
        } else {
            print("- Zero");
        }
        
        // Check if divisible by 3
        if (num % 3 == 0) {
            print("- Divisible by 3");
        } else {
            print("- Not divisible by 3");
        }
        
        // Check if divisible by 5
        if (num % 5 == 0) {
            print("- Divisible by 5");
        } else {
            print("- Not divisible by 5");
        }
        
        // Check range
        if (num >= 1 && num <= 10) {
            print("- Between 1 and 10");
        } else if (num >= 11 && num <= 100) {
            print("- Between 11 and 100");
        } else if (num > 100) {
            print("- Greater than 100");
        } else {
            print("- Less than 1");
        }
    } else if (choice == 3) {
        // Simple calculator
        print("\n=== Simple Calculator ===");
        print("Enter first number: ");
        let first: int = input();
        print("Enter operation (1=+, 2=-, 3=*, 4=/): ");
        let op: int = input();
        print("Enter second number: ");
        let second: int = input();
        
        let calc_result: int = 0;
        if (op == 1) {
            calc_result = first + second;
            print("Result: ", first, " + ", second, " = ", calc_result);
        } else if (op == 2) {
            calc_result = first - second;
            print("Result: ", first, " - ", second, " = ", calc_result);
        } else if (op == 3) {
            calc_result = first * second;
            print("Result: ", first, " * ", second, " = ", calc_result);
        } else if (op == 4) {
            if (second != 0) {
                calc_result = first / second;
                print("Result: ", first, " / ", second, " = ", calc_result);
            } else {
                print("Error: Division by zero!");
            }
        } else {
            print("Invalid operation!");
        }
    } else if (choice == 4) {
        print("Exiting...");
    } else {
        print("Invalid choice!");
    }
    
    // Interactive data collection
    print("\n=== Data Collection ===");
    print("How many items do you want to enter? ");
    let count: int = input();
    
    if (count > 0 && count <= 20) {
        let total: int = 0;
        let max_val: int = 0;
        let min_val: int = 0;
        let first_input: int = 1;
        
        let i: int = 0;
        while (i < count) {
            let display_i: int = i + 1;
            print("Enter item ", display_i, ": ");
            let item: int = input();
            
            total = total + item;
            
            if (first_input == 1) {
                max_val = item;
                min_val = item;
                first_input = 0;
            } else {
                if (item > max_val) {
                    max_val = item;
                }
                if (item < min_val) {
                    min_val = item;
                }
            }
            
            i = i + 1;
        }
        
        print("Statistics:");
        print("- Total: ", total);
        print("- Maximum: ", max_val);
        print("- Minimum: ", min_val);
        let avg: int = total / count;
        print("- Average: ", avg);
    } else {
        print("Invalid count! Must be between 1 and 20.");
    }
    
    // Decision tree example
    print("\n=== Decision Tree ===");
    print("Answer questions with 1 for yes, 0 for no");
    print("Do you like programming? ");
    let q1: int = input();
    
    if (q1 == 1) {
        print("Do you prefer low-level languages? ");
        let q2: int = input();
        if (q2 == 1) {
            print("You might like C or Assembly!");
        } else {
            print("You might like Python or JavaScript!");
        }
    } else {
        print("Do you like math? ");
        let q3: int = input();
        if (q3 == 1) {
            print("You might like data science!");
        } else {
            print("You might like design or art!");
        }
    }
    
    // Input validation loop
    print("\n=== Input Validation ===");
    print("Enter a number between 1 and 5: ");
    let valid_input: int = input();
    
    while (valid_input < 1 || valid_input > 5) {
        print("Invalid! Please enter a number between 1 and 5: ");
        valid_input = input();
    }
    
    print("Thank you! You entered: ", valid_input);
    
    // Pattern matching with input
    print("\n=== Pattern Matching ===");
    print("Enter a number: ");
    let pattern_num: int = input();
    
    if (pattern_num == 0) {
        print("Zero");
    } else if (pattern_num == 1) {
        print("One");
    } else if (pattern_num == 2) {
        print("Two");
    } else if (pattern_num == 3) {
        print("Three");
    } else if (pattern_num == 4) {
        print("Four");
    } else if (pattern_num == 5) {
        print("Five");
    } else {
        print("Greater than five or negative");
    }
    
    print("\n=== All Tests Complete! ===");
    return 0;
}
