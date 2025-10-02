// Comprehensive FFI Test
// Tests various Windows API functions with different calling conventions and parameter types

extern "cdecl" from "user32.dll" {
    func MessageBoxA(hWnd: int, lpText: string, lpCaption: string, uType: int) -> int;
    func MessageBoxW(hWnd: int, lpText: string, lpCaption: string, uType: int) -> int;
    func GetWindowTextA(hWnd: int, lpString: string, nMaxCount: int) -> int;
}

extern "stdcall" from "kernel32.dll" {
    func GetTickCount() -> int;
    func Sleep(dwMilliseconds: int) -> int;
    func GetCurrentProcessId() -> int;
    func GetLastError() -> int;
}

extern "cdecl" from "kernel32.dll" {
    func GetCurrentThreadId() -> int;
    func GetProcessId(hProcess: int) -> int;
}

func main() -> int {
    print("=== Comprehensive FFI Test ===");
    
    // Test basic functions with no parameters
    let process_id: int = GetCurrentProcessId();
    let thread_id: int = GetCurrentThreadId();
    let tick_count: int = GetTickCount();
    
    print("Process ID:", process_id);
    print("Thread ID:", thread_id);
    print("Tick Count:", tick_count);
    
    // Test functions with parameters
    let sleep_result: int = Sleep(1000);
    print("Sleep result:", sleep_result);
    
    // Test string parameters
    let msg_result: int = MessageBoxA(0, "Hello from Twink FFI!", "FFI Test", 0);
    print("MessageBoxA result:", msg_result);
    
    // Test error handling
    let last_error: int = GetLastError();
    print("Last error:", last_error);
    
    // Test another function with different parameters
    let window_text: int = GetWindowTextA(0, "Test", 4);
    print("GetWindowTextA result:", window_text);
    
    print("=== FFI Test Complete ===");
    return 0;
}
