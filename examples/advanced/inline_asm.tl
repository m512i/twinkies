// ========================================
// Inline Assembly Examples
// Demonstrates inline assembly support in Twink Language
// ========================================

// Example 1: Simple inline assembly - move immediate value to register
func simple_asm() -> int {
    let result: int;
    
    // Move 42 into RAX register, then move RAX to result variable
    asm {
        "mov $42, %%rax\n\t"
        "mov %%rax, %0"
        : "=r" (result)
        : "rax"
    };
    
    return result;
}

// Example 2: Inline assembly with input operands
func add_with_asm(a: int, b: int) -> int {
    let result: int;
    
    // Add two numbers using inline assembly
    // %0 is the output (result)
    // %1 is the first input (a)
    // %2 is the second input (b)
    asm {
        "mov %1, %0\n\t"
        "addq %2, %0"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "cc"
    };
    
    return result;
}

// Example 3: Volatile inline assembly (prevents optimization)
func volatile_asm() -> int {
    let value: int = 0;
    
    // Volatile ensures the assembly is not optimized away
    // This is useful for memory barriers, CPUID, etc.
    asm volatile {
        "mov $100, %0"
        : "=r" (value)
        : 
        : "memory"
    };
    
    return value;
}

// Example 4: Inline assembly with multiple outputs
func get_cpu_info() -> int {
    // CPUID instruction - get CPU information
    // EAX=1 gets processor info and feature flags
    // Note: CPUID uses 32-bit registers (EAX, EBX, ECX, EDX) even on 64-bit
    // We only need EAX for the return value, so we'll use a single output
    let eax: int;
    asm {
        "cpuid"
        : "=a" (eax)
        : "a" (1)
        : "ebx", "ecx", "edx", "cc"
    };
    
    // Return the EAX value (processor signature)
    return eax;
}

// Example 5: Inline assembly with clobbers
func read_timestamp() -> int64 {
    // RDTSC instruction - read time stamp counter
    // Returns 64-bit value in EDX:EAX (32-bit parts even on 64-bit)
    // We'll read both parts and combine them
    // Note: Using separate 32-bit reads to avoid register size mismatch
    let low: int;
    let high: int;
    
    asm {
        "rdtsc"
        : "=a" (low), "=d" (high)
        :
        : "cc"
    };
    
    // Combine high and low into 64-bit value
    // Shift high 32 bits left and OR with low
    // Note: This requires proper 64-bit arithmetic which the compiler handles
    let result: int64 = low;
    // For now, return low 32 bits (full combination requires int64 shift support)
    return result;
}

// Example 6: Inline assembly for bit manipulation
func set_bit(value: int, bit: int) -> int {
    let result: int = value;
    
    // Set a specific bit using BTS (Bit Test and Set)
    asm {
        "bts %1, %0"
        : "+r" (result)
        : "r" (bit)
        : "cc"
    };
    
    return result;
}

// Example 7: Inline assembly with memory operands
func memory_operation(ptr: int) -> int {
    let value: int;
    
    // Load value from memory address
    // Note: This is a simplified example - real usage would need proper pointer handling
    asm {
        "mov (%1), %0"
        : "=r" (value)
        : "r" (ptr)
        : "memory"
    };
    
    return value;
}

// Example 8: Simple arithmetic operations
func multiply_asm(a: int, b: int) -> int {
    let result: int;
    
    // Multiply using IMUL instruction
    asm {
        "mov %1, %0\n\t"
        "imulq %2, %0"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "cc"
    };
    
    return result;
}

// Example 9: Compare and conditional operations
func compare_asm(a: int, b: int) -> int {
    let result: int;
    
    // Compare two values and set result based on comparison
    // Use %b0 to write directly to the low byte of the result register
    // This avoids the movzbl issue with 64-bit registers
    asm {
        "cmpq %2, %1\n\t"
        "setg %b0"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "cc"
    };
    
    return result;
}

// Example 10: String length using inline assembly (simplified)
func string_length_asm(str: int) -> int {
    let length: int;
    
    // SCASB instruction to find string length
    // This is a simplified example using 64-bit registers
    // SCASB scans for the byte in AL (0x00 = null terminator)
    asm {
        "mov %1, %%rdi\n\t"
        "mov $0xFFFFFFFFFFFFFFFF, %%rcx\n\t"
        "xor %%rax, %%rax\n\t"
        "repne scasb\n\t"
        "not %%rcx\n\t"
        "dec %%rcx\n\t"
        "mov %%rcx, %0"
        : "=r" (length)
        : "r" (str)
        : "rax", "rcx", "rdi", "cc"
    };
    
    return length;
}

// Example 11: Get CPU name using CPUID
// CPUID with EAX=0x80000002, 0x80000003, 0x80000004 returns the CPU brand string
// Each call returns 16 bytes (4 registers × 4 bytes) for a total of 48 bytes
// The CPU name is stored as ASCII characters across all 12 registers

// Get all registers from CPUID leaf 0x80000002 (first 16 bytes of CPU name)
func get_cpu_name_leaf1_eax() -> int {
    let eax: int;
    let leaf: int = 2147483650; // 0x80000002
    asm {
        "cpuid"
        : "=a" (eax)
        : "a" (leaf)
        : "ebx", "ecx", "edx", "cc"
    };
    return eax;
}

func get_cpu_name_leaf1_ebx() -> int {
    let ebx: int;
    let leaf: int = 2147483650; // 0x80000002
    asm {
        "cpuid"
        : "=b" (ebx)
        : "a" (leaf)
        : "ecx", "edx", "cc"
    };
    return ebx;
}

func get_cpu_name_leaf1_ecx() -> int {
    let ecx: int;
    let leaf: int = 2147483650; // 0x80000002
    asm {
        "cpuid"
        : "=c" (ecx)
        : "a" (leaf)
        : "ebx", "edx", "cc"
    };
    return ecx;
}

func get_cpu_name_leaf1_edx() -> int {
    let edx: int;
    let leaf: int = 2147483650; // 0x80000002
    asm {
        "cpuid"
        : "=d" (edx)
        : "a" (leaf)
        : "ebx", "ecx", "cc"
    };
    return edx;
}

// Get all registers from CPUID leaf 0x80000003 (second 16 bytes of CPU name)
func get_cpu_name_leaf2_eax() -> int {
    let eax: int;
    let leaf: int = 2147483651; // 0x80000003
    asm {
        "cpuid"
        : "=a" (eax)
        : "a" (leaf)
        : "ebx", "ecx", "edx", "cc"
    };
    return eax;
}

func get_cpu_name_leaf2_ebx() -> int {
    let ebx: int;
    let leaf: int = 2147483651; // 0x80000003
    asm {
        "cpuid"
        : "=b" (ebx)
        : "a" (leaf)
        : "ecx", "edx", "cc"
    };
    return ebx;
}

func get_cpu_name_leaf2_ecx() -> int {
    let ecx: int;
    let leaf: int = 2147483651; // 0x80000003
    asm {
        "cpuid"
        : "=c" (ecx)
        : "a" (leaf)
        : "ebx", "edx", "cc"
    };
    return ecx;
}

func get_cpu_name_leaf2_edx() -> int {
    let edx: int;
    let leaf: int = 2147483651; // 0x80000003
    asm {
        "cpuid"
        : "=d" (edx)
        : "a" (leaf)
        : "ebx", "ecx", "cc"
    };
    return edx;
}

// Get all registers from CPUID leaf 0x80000004 (third 16 bytes of CPU name)
func get_cpu_name_leaf3_eax() -> int {
    let eax: int;
    let leaf: int = 2147483652; // 0x80000004
    asm {
        "cpuid"
        : "=a" (eax)
        : "a" (leaf)
        : "ebx", "ecx", "edx", "cc"
    };
    return eax;
}

func get_cpu_name_leaf3_ebx() -> int {
    let ebx: int;
    let leaf: int = 2147483652; // 0x80000004
    asm {
        "cpuid"
        : "=b" (ebx)
        : "a" (leaf)
        : "ecx", "edx", "cc"
    };
    return ebx;
}

func get_cpu_name_leaf3_ecx() -> int {
    let ecx: int;
    let leaf: int = 2147483652; // 0x80000004
    asm {
        "cpuid"
        : "=c" (ecx)
        : "a" (leaf)
        : "ebx", "edx", "cc"
    };
    return ecx;
}

func get_cpu_name_leaf3_edx() -> int {
    let edx: int;
    let leaf: int = 2147483652; // 0x80000004
    asm {
        "cpuid"
        : "=d" (edx)
        : "a" (leaf)
        : "ebx", "ecx", "cc"
    };
    return edx;
}

// Helper function to print a character using inline assembly
// Uses putchar with Windows x64 calling convention (RCX for first parameter)
func print_char(c: int) -> int {
    // Windows x64: first parameter in RCX, need 32-byte shadow space
    asm {
        "mov %0, %%rcx\n\t"
        "sub $32, %%rsp\n\t"      // Allocate shadow space
        "call putchar\n\t"
        "add $32, %%rsp"           // Restore stack
        :
        : "r" (c)
        : "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "cc", "memory"
    };
    return 0;
}

// Extract and print 4 bytes from a 32-bit integer (little-endian)
// Use inline assembly to extract bytes since Twink doesn't have bitwise operators
func print_int_as_chars(value: int) -> int {
    let byte0: int;
    let byte1: int;
    let byte2: int;
    let byte3: int;
    
    // Extract byte0 (lowest byte) - value & 0xFF
    // Use %k1 to get 32-bit version of the 64-bit register
    asm {
        "movl %k1, %%eax\n\t"
        "andl $0xFF, %%eax"            // byte0 = value & 0xFF
        : "=a" (byte0)
        : "r" (value)
        : "cc"
    };
    
    // Extract byte1 - (value >> 8) & 0xFF
    asm {
        "movl %k1, %%eax\n\t"
        "shrl $8, %%eax\n\t"
        "andl $0xFF, %%eax"
        : "=a" (byte1)
        : "r" (value)
        : "cc"
    };
    
    // Extract byte2 - (value >> 16) & 0xFF
    asm {
        "movl %k1, %%eax\n\t"
        "shrl $16, %%eax\n\t"
        "andl $0xFF, %%eax"
        : "=a" (byte2)
        : "r" (value)
        : "cc"
    };
    
    // Extract byte3 - (value >> 24) & 0xFF
    asm {
        "movl %k1, %%eax\n\t"
        "shrl $24, %%eax\n\t"
        "andl $0xFF, %%eax"
        : "=a" (byte3)
        : "r" (value)
        : "cc"
    };
    
    // Print each byte as a character (skip null bytes)
    if (byte0 != 0) {
        print_char(byte0);
    }
    if (byte1 != 0) {
        print_char(byte1);
    }
    if (byte2 != 0) {
        print_char(byte2);
    }
    if (byte3 != 0) {
        print_char(byte3);
    }
    
    return 0;
}

// Print the full CPU name by extracting bytes from all 12 registers
func print_cpu_name() -> int {
    // Print all bytes from leaf 1 (0x80000002)
    print_int_as_chars(get_cpu_name_leaf1_eax());
    print_int_as_chars(get_cpu_name_leaf1_ebx());
    print_int_as_chars(get_cpu_name_leaf1_ecx());
    print_int_as_chars(get_cpu_name_leaf1_edx());
    
    // Print all bytes from leaf 2 (0x80000003)
    print_int_as_chars(get_cpu_name_leaf2_eax());
    print_int_as_chars(get_cpu_name_leaf2_ebx());
    print_int_as_chars(get_cpu_name_leaf2_ecx());
    print_int_as_chars(get_cpu_name_leaf2_edx());
    
    // Print all bytes from leaf 3 (0x80000004)
    print_int_as_chars(get_cpu_name_leaf3_eax());
    print_int_as_chars(get_cpu_name_leaf3_ebx());
    print_int_as_chars(get_cpu_name_leaf3_ecx());
    print_int_as_chars(get_cpu_name_leaf3_edx());
    
    // Print newline
    print_char(10); // '\n'
    
    return 0;
}

// Main function to demonstrate inline assembly
func main() -> int {
    print(1); // Start marker
    
    // Test simple inline assembly
    let simple_result: int = simple_asm();
    print(simple_result); // Should print 42
    
    // Test addition with inline assembly
    let add_result: int = add_with_asm(10, 20);
    print(add_result); // Should print 30
    
    // Test volatile inline assembly
    let volatile_result: int = volatile_asm();
    print(volatile_result); // Should print 100
    
    // Test multiplication
    let mul_result: int = multiply_asm(6, 7);
    print(mul_result); // Should print 42
    
    // Test comparison
    let cmp_result: int = compare_asm(15, 10);
    print(cmp_result); // Should print 1 (15 > 10)
    
    let cmp_result2: int = compare_asm(5, 10);
    print(cmp_result2); // Should print 0 (5 <= 10)
    
    // Test bit manipulation
    let bit_result: int = set_bit(0, 3);
    print(bit_result); // Should have bit 3 set
    
    // Test CPU info (may vary by system)
    let cpu_info: int = get_cpu_info();
    print(cpu_info);
    
    // Test timestamp
    let timestamp: int64 = read_timestamp();
    print(timestamp);
    
    // Test CPU name - print as readable string
    print(999); // CPU name marker
    print_cpu_name(); // Print the actual CPU name as a readable string
    
    print(0); // End marker
    
    return 0;
}

