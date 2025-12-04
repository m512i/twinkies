// Complete FFI Test - File Writing and MessageBox
extern "cdecl" from "kernel32.dll" {
    func GetTickCount() -> int;
    func GetCurrentProcessId() -> int;
    func GetLastError() -> int;
    func Sleep(dwMilliseconds: int) -> int;
    func CreateFileA(lpFileName: string, dwDesiredAccess: int, dwShareMode: int, lpSecurityAttributes: int, dwCreationDisposition: int, dwFlagsAndAttributes: int, hTemplateFile: int) -> int;
    func CloseHandle(hObject: int) -> int;
    func GetFileSize(hFile: int, lpFileSizeHigh: int) -> int;
    func WriteFile(hFile: int, lpBuffer: string, nNumberOfBytesToWrite: int, lpNumberOfBytesWritten: int, lpOverlapped: int) -> int;
}

extern "stdcall" from "user32.dll" {
    func MessageBoxA(hWnd: int, lpText: string, lpCaption: string, uType: int) -> int;
    func GetDesktopWindow() -> int;
    func GetSystemMetrics(nIndex: int) -> int;
}

extern "cdecl" from "msvcrt.dll" {
    func strlen(str: string) -> int;
    func strcmp(str1: string, str2: string) -> int;
    func atoi(str: string) -> int;
    func rand() -> int;
    func srand(seed: int) -> int;
}

func main() -> int {
    print("=== Complete FFI Test ===");

    // Test system information
    let ticks: int = GetTickCount();
    print("System uptime (ticks):", ticks);

    let process_id: int = GetCurrentProcessId();
    print("Current process ID:", process_id);

    // Test screen information
    let screen_width: int = GetSystemMetrics(0);
    let screen_height: int = GetSystemMetrics(1);
    print("Screen resolution:", screen_width, "x", screen_height);

    // Test random number generation
    srand(ticks);
    let rand1: int = rand();
    let rand2: int = rand();
    print("Random numbers:", rand1, rand2);

    // Test file operations (create and write to a test file)
    print("Testing file operations...");
    let file_handle: int = CreateFileA("test_ffi_output.txt", 1073741824, 0, 0, 2, 128, 0);
    print("File handle:", file_handle);

    if (file_handle != -1) {
        // Write some data to the file
        let test_data: string = "Hello from Twink FFI!\nThis is a test file created by the FFI system.\n";
        let data_length: int = strlen(test_data);
        let bytes_written: int = 0;

        let write_result: int = WriteFile(file_handle, test_data, data_length, bytes_written, 0);
        print("Write result:", write_result);
        print("Bytes written:", bytes_written);

        let file_size: int = GetFileSize(file_handle, 0);
        print("File size:", file_size);

        let close_result: int = CloseHandle(file_handle);
        print("Close file result:", close_result);

        print("File written successfully!");
    } else {
        print("Failed to create file");
    }

    // Test string operations
    let test_string: string = "Complete FFI Test";
    let str_len: int = strlen(test_string);
    print("String length:", str_len);

    let str_cmp: int = strcmp("Complete", "Test");
    print("String comparison:", str_cmp);

    // Test number conversion
    let num: int = atoi("12345");
    print("Number conversion:", num);

    // Test error handling
    let last_error: int = GetLastError();
    print("Last system error:", last_error);

    // Test timing
    let start_time: int = GetTickCount();
    Sleep(500); // Sleep for 500ms
    let end_time: int = GetTickCount();
    let elapsed: int = end_time - start_time;
    print("Elapsed time (ms):", elapsed);

    // Test MessageBox - show a message to the user
    print("Showing MessageBox...");
    let msg_result: int = MessageBoxA(0, "FFI Test Complete!\nCheck test_ffi_output.txt for the written file.", "Twink FFI Test", 0);
    print("MessageBox result:", msg_result);

    print("=== Complete FFI Test Complete ===");
    return 0;
}
