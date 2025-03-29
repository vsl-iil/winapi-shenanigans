// https://gargaj.github.io/demos-for-dummies/#graphics-textures/1

#define _CRT_SECURE_NO_WARNINGS
#include <algorithm>
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

#define MINIAUDIO_IMPLEMENTATION
#include "..\external\miniaudio.h"

#define CGLTF_IMPLEMENTATION
#include "..\external\cgltf.h"

#include "..\external\ccVector.h"

// а так можно было?
// указываем, какие библиотеки компилятор должен слинковать. 
// user32 отвечает за некоторые используемые функции WinAPI,
// gdi32 - за отрисовку окна,
// d3d11 - за взаимодействие с DirectX. 
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"d3dcompiler.lib")

bool gWindowWantsToQuit = false;

// window procedure - процедура, обрабатывающая системные сообщения к окну (сиречь события)
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
                // не даём выключить экран / уйти в скринсейвер
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

    // ответ по умолчанию
    return DefWindowProcA(hWnd, uMsg, wParam, lParam);
}

int main() {
    // имя класса окна
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

    CD3D11_RASTERIZER_DESC rasterizerDesc = CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());
    rasterizerDesc.FrontCounterClockwise = true;
    ID3D11RasterizerState* rasterizerState = NULL;
    device->CreateRasterizerState(&rasterizerDesc, &rasterizerState);

    ///////////////////////// CGLTF ////////////////////////

    cgltf_options gltfOptions;
    memset(&gltfOptions, 0, sizeof(cgltf_options));

    cgltf_data* meshData = NULL;
    const char* meshFilename = "Duck.glb";

    cgltf_result res;
    if ( (res = cgltf_parse_file(&gltfOptions, meshFilename, &meshData)) != cgltf_result_success ) {
        return res+100;
    }
    if ( (res = cgltf_load_buffers(&gltfOptions, meshData, meshFilename)) != cgltf_result_success ) {
        return -res-100;
    }

    cgltf_primitive& primitive = meshData->meshes[0].primitives[0];
    D3D11_BUFFER_DESC bufferDesc = CD3D11_BUFFER_DESC((UINT)primitive.attributes[0].data->buffer_view->size, D3D11_BIND_VERTEX_BUFFER);
    D3D11_SUBRESOURCE_DATA subData = { 0 };
    subData.pSysMem = (char*)primitive.attributes[0].data->buffer_view->buffer->data + primitive.attributes[0].data->buffer_view->offset;
    ID3D11Buffer* vertexBuffer = NULL;

    if (device->CreateBuffer(&bufferDesc, &subData, &vertexBuffer) != S_OK) {
        return 9;
    }

    D3D11_BUFFER_DESC ibBufferDesc = CD3D11_BUFFER_DESC((UINT)primitive.indices->buffer_view->size, D3D11_BIND_INDEX_BUFFER);
    D3D11_SUBRESOURCE_DATA ibSubData = { 0 };
    ibSubData.pSysMem = (char*)primitive.indices->buffer_view->buffer->data + primitive.indices->buffer_view->offset;
    ID3D11Buffer* indexBuffer = NULL;
    if (device->CreateBuffer(&ibBufferDesc, &ibSubData, &indexBuffer) != S_OK) {
        return 10;
    }

    float constantBufferData[32] = { 0 };
    D3D11_BUFFER_DESC constantBufferDesc = CD3D11_BUFFER_DESC(sizeof(constantBufferData), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
    D3D11_SUBRESOURCE_DATA constantSubData = { 0 };
    constantSubData.pSysMem = constantBufferData;
    ID3D11Buffer* constantBuffer = NULL;

    if (device->CreateBuffer(&constantBufferDesc, &constantSubData, &constantBuffer) != S_OK) {
        return 11;
    }
    //////////////////////// SHADERS ////////////////////////

    char shaderSource[] = R"(
      cbuffer c 
      {
        float4x4 projectionMatrix;
        float4x4 worldMatrix;
      };

      struct VS_INPUT
      {
        float3 Pos: POSITION;
      };

      struct VS_OUTPUT
      {
        float4 Pos: SV_POSITION;
      };
      
      VS_OUTPUT vs_main( VS_INPUT In )
      {
        VS_OUTPUT Out;
        Out.Pos = float4( In.Pos, 1.0 );
        Out.Pos = mul(worldMatrix, Out.Pos);
        Out.Pos = mul(projectionMatrix, Out.Pos);
        return Out;
      }

      float4 ps_main( VS_OUTPUT In ): SV_TARGET
      {
        float4 c = 0;
        return c;
      }
    )";

    // vertex shader compile
    ID3DBlob* vertexShaderBlob = NULL;
    if (D3DCompile(shaderSource, strlen(shaderSource), NULL, NULL, NULL, "vs_main", "vs_5_0", 0, 0, &vertexShaderBlob, NULL) != S_OK) {
        return 12;
    }
    ID3D11VertexShader* vertexShader = NULL;
    if (device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), NULL, &vertexShader) != S_OK) {
        return 13;
    }

    // pixel shader compile
    ID3DBlob* pixelShaderBlob = NULL;
    if (D3DCompile(shaderSource, strlen(shaderSource), NULL, NULL, NULL, "ps_main", "ps_5_0", 0, 0, &pixelShaderBlob, NULL) != S_OK) {
        return 14;
    }
    ID3D11PixelShader* pixelShader = NULL;
    if (device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), NULL, &pixelShader) != S_OK) {
        return 15;
    }

    static D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    ID3D11InputLayout* inputLayout = NULL;
    if (device->CreateInputLayout(inputDesc, 1, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &inputLayout) != S_OK) {
        return 16;
    }

    ///////////////////////// AUDIO ////////////////////////
    ma_engine engine;
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        return 20;
    }

    ma_sound sound;
    if (ma_sound_init_from_file(&engine, "audio.mp3", 0, NULL, NULL, &sound) != MA_SUCCESS) {
        return 21;
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
        // const float clearColor[ 4 ] = { 0.1f, 0.2f, 0.3f, 0.0f };
        context->ClearRenderTargetView(backBufferView, clearColor);
        context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

        // DEMO HERE
        mat4x4& projectionMatrix = *(mat4x4*)&constantBufferData[0];
        mat4x4Perspective(projectionMatrix, 3.1415f * 0.25f, width / (float)height, 0.01f, 10.0f);

        mat4x4& worldMatrix = *(mat4x4*)&constantBufferData[16];
        mat4x4Identity(worldMatrix);
        mat4x4RotateY(worldMatrix, cursor);
        const vec3 translation = { 0.0f, 0.0f, -5.0f };
        mat4x4Translate(worldMatrix, translation);

        /////////////////////////

        D3D11_MAPPED_SUBRESOURCE mappedSubRes;
        context->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubRes);
        CopyMemory(mappedSubRes.pData, constantBufferData, constantBufferDesc.ByteWidth);
        context->Unmap(constantBuffer, 0);

        // render model
        // set viewport
        const D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.0f, 0.0f, (float)width, (float)height);
        context->RSSetViewports(1, &viewport);

        // render target - back buffer
        context->OMSetRenderTargets(1, &backBufferView, depthStencilView);

        // constant buffers
        context->VSSetConstantBuffers(0, 1, &constantBuffer);

        // set shaders, rasterizer state & input layout
        context->VSSetShader(vertexShader, NULL, 0);
        context->PSSetShader(pixelShader, NULL, 0);

        context->RSSetState(rasterizerState);
        context->IASetInputLayout(inputLayout);

        // set vertex buffer
        ID3D11Buffer* buffers[] = { vertexBuffer };
        const UINT stride[] = { (UINT)primitive.attributes[0].data->stride };
        const UINT offset[] = { 0 };

        context->IASetVertexBuffers(0, 1, buffers, stride, offset);

        // set index buffer
        DXGI_FORMAT format;
        if (primitive.indices->component_type == cgltf_component_type_r_32u) {
            format = DXGI_FORMAT_R32_UINT;
        } else {
            format = DXGI_FORMAT_R16_UINT;
        }
        context->IASetIndexBuffer(indexBuffer, format, 0);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        context->DrawIndexed((UINT)primitive.indices->count, 0, 0);

        swapchain->Present(0, 0);
    }

    ma_sound_stop(&sound);
    ma_engine_stop(&engine);

    rasterizerState->Release();
    depthStencilView->Release();
    depthStencil->Release();
    backBufferView->Release();
    backBuffer->Release();
    constantBuffer->Release();
    pixelShader->Release();
    vertexShader->Release();
    vertexBuffer->Release();
    indexBuffer->Release();
    inputLayout->Release();

    cgltf_free(meshData);

    context->Release();
    device->Release();

    DestroyWindow(hWnd);
    UnregisterClassA(CLASS_NAME, GetModuleHandle(NULL));

    return 0;
}