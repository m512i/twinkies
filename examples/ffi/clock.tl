extern "stdcall" from "kernel32.dll" {
    func GetLocalTime(lpSystemTime: int) -> int;
    func Sleep(dwMilliseconds: int) -> int;
    func GetTickCount() -> int;
}

extern "cdecl" from "msvcrt.dll" {
    func malloc(size: int) -> int;
    func free(ptr: int) -> int;
    func time(timer: int) -> int;
    func localtime(time_t: int) -> int;
}

extern "stdcall" from "user32.dll" {
    func MessageBoxA(hWnd: int, lpText: string, lpCaption: string, uType: int) -> int;
}

func main() -> int {
    print("=== ACTUAL Real Clock - Real System Time ===");
    print("Press Ctrl+C to stop");
    print("");
    
    print("Clock will update every 5 seconds...\n lol");
    print("");
    
    let update_count: int = 0;
    while (update_count < 24) { 
        let current_time: int = time(0);
        
        let eastern_offset: int = -4 * 3600; 
        let local_time: int = current_time + eastern_offset;
        
        let hours: int = (local_time / 3600) % 24;
        let minutes: int = (local_time / 60) % 60;
        let seconds: int = local_time % 60;
        
        let display_hour: int = hours;
        let am_pm: string = "AM";
        
        if (hours == 0) {
            display_hour = 12; 
        } else if (hours > 12) {
            display_hour = hours - 12; 
            am_pm = "PM";
        } else if (hours == 12) {
            am_pm = "PM"; 
        }
        
        print("Time:", display_hour, ":", minutes, ":", seconds, am_pm);
        Sleep(5000);
        update_count = update_count + 1;
    }
    
    let msg_result: int = MessageBoxA(0, "Real clock session complete!", "ACTUAL Clock", 0);
    print("MessageBox result:", msg_result);
    
    print("=== ACTUAL Clock Complete ===");
    return 0;
}