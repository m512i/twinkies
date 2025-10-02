// Advanced FFI Test - Complex Windows API Integration
extern "cdecl" from "kernel32.dll" {
    func GetTickCount() -> int;
    func Sleep(dwMilliseconds: int) -> int;
    func GetCurrentProcessId() -> int;
    func GetCurrentThreadId() -> int;
    func GetLastError() -> int;
    func CreateMutexA(lpMutexAttributes: int, bInitialOwner: int, lpName: string) -> int;
    func ReleaseMutex(hMutex: int) -> int;
    func CloseHandle(hObject: int) -> int;
    func GetSystemTimeAsFileTime(lpFileTime: int) -> int;
}

extern "stdcall" from "user32.dll" {
    func MessageBoxA(hWnd: int, lpText: string, lpCaption: string, uType: int) -> int;
    func FindWindowA(lpClassName: string, lpWindowName: string) -> int;
    func GetWindowTextLengthA(hWnd: int) -> int;
    func IsWindowVisible(hWnd: int) -> int;
    func GetDesktopWindow() -> int;
    func GetWindow(hWnd: int, uCmd: int) -> int;
}

extern "cdecl" from "msvcrt.dll" {
    func strlen(str: string) -> int;
    func strcpy(dest: string, src: string) -> string;
    func strcmp(str1: string, str2: string) -> int;
    func strcat(dest: string, src: string) -> string;
    func strchr(str: string, c: int) -> string;
}

func main() -> int {
    print("=== Advanced FFI Test - Complex APIs ===");
    
    // Test basic system information
    let ticks: int = GetTickCount();
    print("System tick count:", ticks);
    
    let process_id: int = GetCurrentProcessId();
    print("Process ID:", process_id);
    
    let thread_id: int = GetCurrentThreadId();
    print("Thread ID:", thread_id);
    
    // Test string operations
    let str_len: int = strlen("Advanced FFI Test");
    print("String length:", str_len);
    
    let str_cmp: int = strcmp("Hello", "Hello");
    print("String comparison (Hello vs Hello):", str_cmp);
    
    let str_cmp2: int = strcmp("Hello", "World");
    print("String comparison (Hello vs World):", str_cmp2);
    
    // Test window operations
    let desktop_window: int = GetDesktopWindow();
    print("Desktop window handle:", desktop_window);
    
    if (desktop_window != 0) {
        let text_length: int = GetWindowTextLengthA(desktop_window);
        print("Desktop window text length:", text_length);
        
        let is_visible: int = IsWindowVisible(desktop_window);
        print("Desktop window is visible:", is_visible);
    }
    
    // Test mutex operations
    print("Testing mutex operations...");
    let mutex_handle: int = CreateMutexA(0, 0, "TwinkFFITestMutex");
    print("Mutex handle:", mutex_handle);
    
    if (mutex_handle != 0) {
        let release_result: int = ReleaseMutex(mutex_handle);
        print("Release mutex result:", release_result);
        
        let close_result: int = CloseHandle(mutex_handle);
        print("Close mutex handle result:", close_result);
    }
    
    // Test error handling
    let last_error: int = GetLastError();
    print("Last system error:", last_error);
    
    // Test system time (using a safer approach)
    let time_result: int = GetTickCount();
    print("System time (tick count):", time_result);
    
    // Test sleep with feedback
    print("Sleeping for 2 seconds...");
    let sleep_result: int = Sleep(2000);
    print("Sleep completed, result:", sleep_result);
    
    // Test MessageBox (this will show a popup)
    print("Showing message box...");
    let msg_result: int = MessageBoxA(0, "Advanced FFI Test Complete!", "Twink FFI", 0);
    print("Message box result:", msg_result);
    
    print("=== Advanced FFI Test Complete ===");
    return 0;
}