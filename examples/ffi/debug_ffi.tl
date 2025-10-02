// Simple FFI Debug Test
extern "cdecl" from "kernel32.dll" {
    func GetTickCount() -> int;
}

extern "cdecl" from "msvcrt.dll" {
    func strlen(str: string) -> int;
}

func main() -> int {
    print("=== Debug FFI Test ===");
    
    let ticks: int = GetTickCount();
    print("Tick count:", ticks);
    
    let str_len: int = strlen("Hello World");
    print("String length:", str_len);
    
    print("=== Debug Test Complete ===");
    return 0;
}
