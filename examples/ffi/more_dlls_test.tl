// FFI Test with Multiple DLLs
extern "cdecl" from "kernel32.dll" {
    func GetTickCount() -> int;
    func GetCurrentProcessId() -> int;
}

extern "stdcall" from "user32.dll" {
    func MessageBoxA(hWnd: int, lpText: string, lpCaption: string, uType: int) -> int;
    func GetSystemMetrics(nIndex: int) -> int;
}

extern "cdecl" from "msvcrt.dll" {
    func strlen(str: string) -> int;
    func rand() -> int;
    func srand(seed: int) -> int;
}

// Advanced Windows API
extern "stdcall" from "advapi32.dll" {
    func RegOpenKeyExA(hKey: int, lpSubKey: string, ulOptions: int, samDesired: int, phkResult: int) -> int;
    func RegCloseKey(hKey: int) -> int;
}

// Shell functions
extern "stdcall" from "shell32.dll" {
    func ShellExecuteA(hwnd: int, lpOperation: string, lpFile: string, lpParameters: string, lpDirectory: string, nShowCmd: int) -> int;
}

// Graphics functions
extern "stdcall" from "gdi32.dll" {
    func GetPixel(hdc: int, nXPos: int, nYPos: int) -> int;
    func SetPixel(hdc: int, x: int, y: int, color: int) -> int;
}

func main() -> int {
    print("=== Multiple DLLs FFI Test ===");
    
    // Test basic functions
    let ticks: int = GetTickCount();
    print("System uptime:", ticks);
    
    let process_id: int = GetCurrentProcessId();
    print("Process ID:", process_id);
    
    // Test screen metrics
    let screen_width: int = GetSystemMetrics(0);
    let screen_height: int = GetSystemMetrics(1);
    print("Screen resolution:", screen_width, "x", screen_height);
    
    // Test string functions
    let str_len: int = strlen("Multiple DLLs Test");
    print("String length:", str_len);
    
    // Test random numbers
    srand(ticks);
    let rand_num: int = rand();
    print("Random number:", rand_num);
    
    // Test registry functions (these might fail but show the concept)
    print("Testing registry functions...");
    let reg_result: int = RegOpenKeyExA(0x80000000, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", 0, 0x20019, 0);
    print("Registry open result:", reg_result);
    
    // Test shell functions
    print("Testing shell functions...");
    let shell_result: int = ShellExecuteA(0, "open", "notepad.exe", "", "", 1);
    print("Shell execute result:", shell_result);
    
    // Test graphics functions (these might fail but show the concept)
    print("Testing graphics functions...");
    let pixel_color: int = GetPixel(0, 100, 100);
    print("Pixel color at (100,100):", pixel_color);
    
    // Show message box
    let msg_result: int = MessageBoxA(0, "Multiple DLLs FFI Test Complete!", "Twink FFI", 0);
    print("MessageBox result:", msg_result);
    
    print("=== Multiple DLLs Test Complete ===");
    return 0;
}
