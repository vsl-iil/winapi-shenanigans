/*
 * Эта программа создаёт окно без рамки с заданной при помощи полигона формой.
 * Точки были заданы мной вручную; теоретически, этот процесс можно упростить 
 * с помощью какой-нибудь библиотеки, позволяющей извлечь координаты точек из 
 * векторного изображения, однако элементы изображения вроде дуг/кривых не будут
 * перенесены - для более сложных фигур имеет смысл использовать маскирование,
 * см. http://comrade.ownz.com/docs/shapewnd.html и файл custom-shape2.cpp
 *
 * les, 2025
 */

#include <Windows.h>
#include <vector>

HRGN hRegion;

HRGN RegionFromPoints(POINT* pts, int pts_len) {
    return CreatePolygonRgn(pts, pts_len, WINDING);
}

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

void ScaleUpPoints(std::vector<POINT>* pts, int scale) {
    for (POINT& point : *pts) {
        point.x *= scale;
        point.y *= scale;
    }
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // Кастомная форма для окна
    std::vector<POINT> pts = {
        POINT {0, 0},
        POINT {8, 0},
        POINT {10, 2},
        POINT {11, 1},
        POINT {10, 0},
        POINT {14, 0},
        POINT {13, 1},
        POINT {14, 2},
        POINT {16, 0},
        POINT {24, 0},
        POINT {12, 8}
    };

    // Отмасштабируем точки
    ScaleUpPoints(&pts, 25);

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

    // Задаём кастомную форму региона
    hRegion = RegionFromPoints(&pts[0], pts.size());
    SetWindowRgn(hwnd, hRegion, TRUE);

    ShowWindow(hwnd, nCmdShow);

    // message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}