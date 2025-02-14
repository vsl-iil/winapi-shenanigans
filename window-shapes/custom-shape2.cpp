#include <Windows.h>

HRGN hRegion;

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

                FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW+1));
                //FillRgn(hdc, hRegion, (HBRUSH)(COLOR_WINDOW)+1);

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

    ShowWindow(hwnd, nCmdShow);

    // message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;

}