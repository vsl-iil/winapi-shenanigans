// https://gargaj.github.io/demos-for-dummies/#graphics-meshload/1

#include <algorithm>
#include <Windows.h>
#include <d3d11.h>
#include <d3dcommon.h>
#include <dxgi.h>
#include <dxgiformat.h>
#define MINIAUDIO_IMPLEMENTATION
#include "..\external\miniaudio.h"

// а так можно было?
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"d3d11.lib")

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

    //////////////////////// DIRECTX ////////////////////////
    DXGI_SWAP_CHAIN_DESC desc = { 0 };
    desc.BufferCount = 2;
    desc.BufferDesc.Width = width;
    desc.BufferDesc.Height = height;
    desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    desc.OutputWindow = hWnd;
    desc.SampleDesc.Count = 1;
    desc.Windowed = true;

    ID3D11Device* device = NULL;
    ID3D11DeviceContext* context = NULL;
    IDXGISwapChain* swapchain = NULL;

    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &desc, &swapchain, &device, NULL, &context) != S_OK) {
        return 4;
    }

    ID3D11Texture2D* backBuffer = NULL;
    ID3D11RenderTargetView* backBufferView = NULL;
    swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    device->CreateRenderTargetView(backBuffer, NULL, &backBufferView);

    D3D11_TEXTURE2D_DESC depthDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R24G8_TYPELESS, width, height, 1, 1, D3D11_BIND_DEPTH_STENCIL);
    ID3D11Texture2D* depthStencil = NULL;
    if (device->CreateTexture2D(&depthDesc, NULL, &depthStencil) != S_OK) {
        return 5;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = CD3D11_DEPTH_STENCIL_VIEW_DESC(D3D11_DSV_DIMENSION_TEXTURE2D, DXGI_FORMAT_D24_UNORM_S8_UINT);
    ID3D11DepthStencilView* depthStencilView = NULL;
    if (device->CreateDepthStencilView(depthStencil, &descDSV, &depthStencilView) != S_OK) {
        return 6;
    }
    ///////////////////////// AUDIO ////////////////////////
    ma_engine engine;
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        return 10;
    }

    ma_sound sound;
    if (ma_sound_init_from_file(&engine, "audio.mp3", 0, NULL, NULL, &sound) != MA_SUCCESS) {
        return 11;
    }

    ma_sound_set_looping(&sound, MA_TRUE);
    ma_sound_start(&sound);
    ///////////////////////////////////////////////////////////

    float total = 0.0f;
    ma_sound_get_length_in_seconds(&sound, &total);
    while (!gWindowWantsToQuit) {
        MSG msg;
        if (PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    
        float cursor = 0.0f;
        ma_sound_get_cursor_in_seconds(&sound, &cursor);
        float red   = (std::max)(0.0f, (std::min)(6.0f * (float)fabs((cursor/total)-0.5f) - 1.0f, 1.0f));
        float green = (std::max)(0.0f, (std::min)(-(6.0f * (float)fabs((cursor/total)-0.33f) - 2.0f), 1.0f));
        float blue  = (std::max)(0.0f, (std::min)(-(6.0f * (float)fabs((cursor/total)-0.67f) - 2.0f), 1.0f));

        const float clearColor[4] = {red, green, blue, 0.0f};
        context->ClearRenderTargetView(backBufferView, clearColor);
        context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

        swapchain->Present(0, 0);
    }

    ma_sound_stop(&sound);
    ma_engine_stop(&engine);

    depthStencilView->Release();
    depthStencil->Release();
    backBufferView->Release();
    backBuffer->Release();
    context->Release();
    device->Release();

    DestroyWindow(hWnd);
    UnregisterClassA(CLASS_NAME, GetModuleHandle(NULL));

    return 0;
}