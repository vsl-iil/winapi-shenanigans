#include <Windows.h>
#include <minwindef.h>
#include <winnt.h>
#include <winuser.h>

#pragma comment(lib,"user32.lib")

INT APIENTRY DllMain(HMODULE hDLL, DWORD Reason, LPVOID Reserved) {
    switch (Reason) {
        case DLL_PROCESS_ATTACH:
            MessageBox(NULL, "Hello world!", "Simple injection", 0);
            break;
        case DLL_PROCESS_DETACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}