// FFI Example: Opening Windows Calculator
extern "cdecl" from "shell32.dll" {
    func ShellExecuteA(hwnd: int, lpOperation: string, lpFile: string, lpParameters: string, lpDirectory: string, nShowCmd: int) -> int;
}

extern "cdecl" from "kernel32.dll" {
    func GetTickCount() -> int;
    func Sleep(dwMilliseconds: int) -> int;
}

func main() -> int {
    print("=== FFI Calculator Launcher ===");

    // Get current time before launching
    let start_time: int = GetTickCount();
    print("Start time (ticks):", start_time);

    print("Launching Windows Calculator...");

    // Launch calculator using ShellExecuteA
    // Parameters: hwnd=0 (no parent), operation="open", file="calc.exe",
    // parameters="", directory="", show command=1 (normal window)
    let result: int = ShellExecuteA(0, "open", "calc.exe", "", "", 1);

    print("ShellExecuteA result:", result);

    // Check if successful (result > 32 means success)
    if (result > 32) {
        print("Calculator launched successfully!");

        // Wait a moment to let calculator start
        print("Waiting 2 seconds...");
        let sleep_result: int = Sleep(2000);
        print("Sleep completed, result:", sleep_result);

        // Get time after launch
        let end_time: int = GetTickCount();
        print("End time (ticks):", end_time);

        let elapsed: int = end_time - start_time;
        print("Time elapsed (ticks):", elapsed);

    } else {
        print("❌ Failed to launch calculator!");
        print("Error codes:");
        print("  0 = Out of memory");
        print("  2 = File not found");
        print("  3 = Path not found");
        print("  5 = Access denied");
        print("  8 = Out of memory");
        print("  26 = Sharing violation");
        print("  27 = Filename association incomplete");
        print("  28 = DDE timeout");
        print("  29 = DDE fail");
        print("  30 = DDE busy");
        print("  31 = No association");
        print("  32 = DLL not found");
    }

    print("=== Calculator Launcher Complete ===");
    return 0;
}
