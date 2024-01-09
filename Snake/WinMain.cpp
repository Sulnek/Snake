#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <d2d1_3.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <cwchar>

#include "Paint.h"
#include "Snake.h"


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


using D2D1::RenderTargetProperties;
using D2D1::HwndRenderTargetProperties;
using D2D1::SizeU;

Snake* snake = nullptr;
Paint* paint = nullptr;

// if something doesn't work, please try changing CALLBACK to WINAPI
// whenever I changed though i got a Warning 
int CALLBACK wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR,
    _In_ int nShowCmd) {
    // Register the window class.
    const wchar_t CLASS_NAME[] = L"Sample Window Class";

    if (hPrevInstance) {
        // According to documentation this should always be 0
        return 1;
    }

    WNDCLASS wc = { };

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    HCURSOR hc = (HCURSOR)LoadImage(NULL, IDC_ARROW, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_SHARED);
    if (hc != nullptr) {
        wc.hCursor = hc;
    }

    ATOM ret = RegisterClass(&wc);
    if (!ret) {
        return 1;
    }

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Snake",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        0, 0, WIN_WIDTH + 15, WIN_HEIGHT + 40,

        nullptr,       // Parent window    
        nullptr,       // Menu
        hInstance,  // Instance handle
        nullptr        // Additional application data
    );

    if (hwnd == nullptr) {
        return 1; // error creating the window
    }

    ShowWindow(hwnd, nShowCmd);
    paint = new Paint();
    if (paint->createResources(hwnd) == 1) {
        return 1;
    }
    snake = new Snake(paint);

    auto clock = std::chrono::system_clock::now();
    auto since_epoch = clock.time_since_epoch();
    auto milis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);
    long long time = milis.count();
    snake->last_time = time;

    // Run the message loop.

    MSG msg = { };
    INT retGetMess = GetMessage(&msg, nullptr, 0, 0);
    while (retGetMess > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        retGetMess = GetMessage(&msg, nullptr, 0, 0);
    }

    if (retGetMess == -1) {
        return 1;
    }

    delete snake;
    delete paint;
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_TIMER:
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;

    case WM_KEYDOWN:
        if (wParam == VK_RIGHT && !snake->orientation_changed) {
            snake->new_orientation = (snake->orientation + 1) % 4;
            snake->orientation_changed = true;
        }
        if (wParam == VK_LEFT && !snake->orientation_changed) {
            snake->orientation_changed = true;
            snake->new_orientation = (snake->orientation + 3) % 4;
        }
        if (wParam == 0x52 && !snake->running) { // "R" 
            snake->restart();
        }
        return 0;

    case WM_PAINT:
    {
        paint->beginDraw();
 
        if (snake->running) {
            paint->drawBgBitmap();
            paint->drawBorders(BOARDER_WIDTH);
            auto clock = std::chrono::system_clock::now();
            auto since_epoch = clock.time_since_epoch();
            auto milis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);
            long long time = milis.count();
            long long time_diff = time - snake->last_time;
            if (time_diff > 1000 * SPEED) {
                snake->moveOneStep();
                snake->last_time = time;
            }
            HRESULT hr = snake->draw();
            if (FAILED(hr)) {
                return 1;
            }
        }
        else {
            paint->setBackground(D2D1::ColorF(0.8f, 0.8f, 0.8f));
            paint->drawLogo();
            wchar_t text[45] = L"Kliknij R aby zrestartować\nUzyskany wynik: ";
            int bufferSize = swprintf(nullptr, 0, L"%s%d", text, abs(snake->len));
            WCHAR* formattedText = new WCHAR[bufferSize + 1];
            swprintf(formattedText, bufferSize + 1, L"%s%d", text, abs(snake->len));

            paint->writeText(formattedText, D2D1::ColorF(0.0f, 0.0f, 0.0f), bufferSize, MARGIN, MARGIN);
        }

        if (paint->endDraw(hwnd)) {
            return 1; // restoring render target
        }
    }
    return 0;

    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}