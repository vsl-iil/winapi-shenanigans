// https://gargaj.github.io/demos-for-dummies/#intro/1

#include <Windows.h>
#include <d3d11.h>
#define MINIAUDIO_IMPLEMENTATION
#include "..\external\miniaudio.h"

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
    windowClass.lpfnWndProc = &WndProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.hIcon = NULL;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = NULL;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = CLASS_NAME;

    if (!RegisterClassA(&windowClass)) {
        // printf("RegisterClassA failed because %ld", GetLastError());
        return 420;
    }

    const int width = 1280;
    const int height = 720;
    const DWORD exStyle = WS_EX_APPWINDOW;
    const DWORD style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
    RECT windowRect = {0, 0, width, height};
    AdjustWindowRectEx(&windowRect, style, FALSE, exStyle);

    HWND hWnd = CreateWindowExA(exStyle, CLASS_NAME, "Demos for Dummies", style, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, NULL, NULL, windowClass.hInstance, NULL);

    if (!hWnd) {
        return 2;
    }

    ///////////////////////// AUDIO ////////////////////////
    ma_engine engine;
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        return 3;
    }

    ma_sound sound;
    if (ma_sound_init_from_file(&engine, "audio.mp3", 0, NULL, NULL, &sound) != MA_SUCCESS) {
        return 4;
    }

    ma_sound_set_looping(&sound, MA_TRUE);
    ma_sound_start(&sound);
    ///////////////////////////////////////////////////////////

    while (!gWindowWantsToQuit) {
        MSG msg;
        if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    
        float cursor = 0.0f;
        ma_sound_get_cursor_in_seconds(&sound, &cursor);
    }

    ma_sound_stop(&sound);
    ma_engine_stop(&engine);

    DestroyWindow(hWnd);
    UnregisterClassA(CLASS_NAME, GetModuleHandle(NULL));

    return 0;
}