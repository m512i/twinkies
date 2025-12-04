# Senior C Developer Interview Questions

This document contains 10 interview questions for senior-level C developers. These questions focus on practical aspects of C programming without delving into low-level concepts like atomics or pthreads.

## Technical Questions

1. **Memory Management**: Explain the different allocation strategies in C (stack vs heap). What are the trade-offs, and how do you decide which to use in different scenarios? Discuss common memory-related bugs and how you prevent/detect them.

2. **Macro Usage**: Describe scenarios where you would use macros in C, along with their advantages and disadvantages. How do you write safe macros that avoid common pitfalls like multiple evaluation?

3. **Optimization Techniques**: What strategies do you use to optimize C code? Discuss compiler optimizations, data structure choices, and algorithm improvements you've implemented in real projects.

4. **Error Handling**: Describe your approach to error handling in C. How do you balance robustness with code readability? What patterns have you found most effective?

5. **Build Systems**: Explain your experience with C build systems and toolchains. How would you set up a complex C project with multiple dependencies? What's your experience with cross-platform compilation?

6. **Testing Strategies**: What testing methodologies do you apply to C codebases? Discuss unit testing, integration testing, and any test-driven development approaches you've used with C.

7. **Undefined Behavior**: What is undefined behavior in C? Provide some common examples and explain how you write code to avoid it. How do you detect undefined behavior in existing code?

8. **C Standard Library**: Discuss the limitations of the C standard library and how you work around them. When do you choose to use standard library functions versus implementing your own?

## Coding Problems

### Problem 1: Custom Memory Pool Allocator

Implement a simple memory pool allocator in C that:

- Pre-allocates a fixed-size memory pool at initialization
- Supports allocating memory blocks of different sizes
- Properly handles alignment requirements
- Can free previously allocated blocks and make them available for reuse
- Includes proper error handling for out-of-memory conditions

Your solution should include:
- Data structures for tracking memory usage
- Functions for initialization, allocation, and deallocation
- Strategies for minimizing fragmentation

### Problem 2: LRU Cache Implementation

Implement an LRU (Least Recently Used) cache in C with the following requirements:

- Support for a configurable maximum size
- O(1) average time complexity for lookup, insert, and delete operations
- Thread-safety is not required (no need for synchronization)
- Handle collision resolution if using hash tables
- Include proper error handling

Your implementation should:
- Define appropriate data structures
- Implement functions for creating/destroying the cache
- Provide functions to insert, retrieve, and remove items
- Include logic to evict the least recently used items when the cache is full

## Evaluation Criteria

Candidates will be evaluated on:
- Code quality and organization
- Error handling and edge cases
- Memory management
- Algorithm efficiency
- Problem-solving approach
- Communication skills while coding
- Testing strategies