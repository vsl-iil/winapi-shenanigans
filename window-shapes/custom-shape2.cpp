#include <Windows.h>
#include <iostream>
#include <vector>
#include <wingdi.h>
#include "..\external\lodepng.h"

HRGN hRegion;
std::vector<unsigned char> image;
unsigned int width, height;

// Процедура-болванка для обработки сообщений для окна
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        // Выходим при закрытии программы
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        
        // Отрисовка окна; это сообщение отправляется системой автоматически
        case WM_PAINT:
            {
                PAINTSTRUCT ps;
                HDC hdc = BeginPaint(hWnd, &ps);

                //FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW+1));
                for (int i = 0; i < image.size(); i += 4) {
                    SetPixel(hdc, (i/4) % width, (i/4) / width, RGB(image[i], image[i+1], image[i+2]));
                }

                EndPaint(hWnd, &ps);
            }
            return 0;
    }

    // Обработка остальных сообщений - по умолчанию:
    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // Регистрируем класс окна
    const char CLASS_NAME[] = "Custom-Shaped Window Class";

    WNDCLASS wc = {};

    wc.lpfnWndProc      = WindowProc;
    wc.hInstance        = hInstance;
    wc.lpszClassName    = CLASS_NAME;

    RegisterClass(&wc);

    // Создаем окно
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "Windows text 123",
        NULL,

        // Размер и положение
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 1;
    }

    // Убираем рамку
    LONG lStyle = GetWindowLong(hwnd, GWL_STYLE);
    lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
    SetWindowLong(hwnd, GWL_STYLE, lStyle);

    // Задаём кастомную форму окна

    // Загружаем изображение с диска:
    unsigned int error;
    if ((error = lodepng::decode(image, width, height, "head.png"))) {
        std::cout << "Image load error: " << lodepng_error_text(error) << std::endl;
        return -1; 
    }

    HRGN hRgn = CreateRectRgn(0, 0, width, height);
    for (int i = 0; i < image.size(); i += 4) {
        // if ((image[i] ^ 255) && (image[i+1] & 255) && (image[i+2] ^ 255) && (image[i+3] ^ 255)) {
        if ((image[i] == 255) && (image[i+1] == 0) && (image[i+2] == 255)) {
            int x = (i/4) % width;
            int y = (i/4) / width;
            HRGN tempRng = CreateRectRgn(x, y, x+1, y+1);
            CombineRgn(hRgn, hRgn, tempRng, RGN_DIFF);
            DeleteObject(tempRng);
        }
    }

    SetWindowRgn(hwnd, hRgn, TRUE);

    ShowWindow(hwnd, nCmdShow);

    // message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // убираем за собой
    DestroyWindow(hwnd);
    UnregisterClassA(CLASS_NAME, hInstance);

    return 0;

}