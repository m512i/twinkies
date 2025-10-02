// Simple FFI Test - Working Example
extern "cdecl" from "kernel32.dll" {
    func GetTickCount() -> int;
    func GetCurrentProcessId() -> int;
    func GetCurrentThreadId() -> int;
}

extern "cdecl" from "msvcrt.dll" {
    func strlen(str: string) -> int;
    func strcmp(str1: string, str2: string) -> int;
}

func main() -> int {
    print("=== Simple FFI Test ===");
    
    // Test basic system functions
    let ticks: int = GetTickCount();
    print("System tick count:", ticks);
    
    let process_id: int = GetCurrentProcessId();
    print("Process ID:", process_id);
    
    let thread_id: int = GetCurrentThreadId();
    print("Thread ID:", thread_id);
    
    // Test string functions
    let str_len: int = strlen("Hello World");
    print("String length:", str_len);
    
    let str_cmp: int = strcmp("Hello", "World");
    print("String comparison (Hello vs World):", str_cmp);
    
    let str_cmp2: int = strcmp("Hello", "Hello");
    print("String comparison (Hello vs Hello):", str_cmp2);
    
    print("=== FFI Test Complete ===");
    return 0;
}
