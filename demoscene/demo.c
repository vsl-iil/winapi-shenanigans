// https://gargaj.github.io/demos-for-dummies/#intro/1

#include <Windows.h>
#include <stdbool.h>
#include <stdio.h>

bool gWindowWantsToQuit = false;

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_KEYDOWN:
        {
            switch (wParam) {
                case VK_ESCAPE:
                    gWindowWantsToQuit = true;
                    break;
            }
        }
        break;

        case WM_SYSCOMMAND:
        {
            switch (wParam) {
                case SC_SCREENSAVE:
                case SC_MONITORPOWER:
                    return 0;
                    break;
            }
        }
        break;

        case WM_CLOSE:
            gWindowWantsToQuit = true;
            break;
    }

    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

int main() {
    const char* CLASS_NAME = "DemosForDummies window class";

    WNDCLASSA windowClass;
    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.hIcon = NULL;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = NULL;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = CLASS_NAME;

    if (!RegisterClassA(&windowClass)) {
        printf("RegisterClassA failed because %ld", GetLastError());
        return 1;
    }

    const int width = 1280;
    const int height = 720;
    const DWORD exStyle = WS_EX_APPWINDOW;
    const DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    RECT windowRect = {0, 0, width, height};
    AdjustWindowRectEx(&windowRect, style, FALSE, exStyle);

    HWND hWnd = CreateWindowExA(exStyle, CLASS_NAME, "Demos for Dummies", style, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, windowClass.hInstance, NULL);

    if (!hWnd) {
        printf("CreateWindow failed because %ld", GetLastError());
        return 2;
    }

    while (!gWindowWantsToQuit) {
        MSG msg;
        if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    DestroyWindow(hWnd);
    UnregisterClassA(CLASS_NAME, GetModuleHandle(NULL));

    return 0;
}