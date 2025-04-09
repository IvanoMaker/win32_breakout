#include <Windows.h>
#include <ctime>

// Constants
constexpr int SCREEN_H = 720;
constexpr int SCREEN_W = 1280;
constexpr int FRAME_DELAY = 16;

// Window procedure for handling messages
LRESULT CALLBACK WinProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            // Clear screen with black
            RECT screenRect;
            GetClientRect(hWnd, &screenRect);
            drawRect(hdc, screenRect, 0);

            EndPaint(hWnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    // Register window class
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WinProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "PaddleGameClass";

    RegisterClassA(&wc);

    // Create the main window
    HWND hWnd = CreateWindowA(
        "PaddleGameClass",
        "Breakout",
        WS_CAPTION | WS_POPUP | WS_SYSMENU,
        50, 50, SCREEN_W, SCREEN_H,
        nullptr, nullptr,
        hInstance, nullptr
    );

    ShowWindow(hWnd, nCmdShow);

    // Game loop
    MSG msg;
    std::clock_t lastTime = std::clock();

    for (;;) {
        while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return 0;
            }
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }

        std::clock_t currentTime = std::clock();
        if ((currentTime - lastTime) * 1000 / CLOCKS_PER_SEC >= FRAME_DELAY) {

            // do game things
          
            InvalidateRect(hWnd, nullptr, TRUE); // Trigger redraw
            lastTime = currentTime;
        }
    }

    return 0;
}
