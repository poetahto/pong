#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define WIDTH 800
#define HEIGHT 600

LRESULT CALLBACK WinProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE instance, HINSTANCE prevInstance, PSTR cmd, int cmdShow) {
    WNDCLASSW windowClass = {
        .lpszClassName = L"PongWindowClass",
        .lpfnWndProc = WinProc,
        .hInstance = instance,
    };
    
    ATOM atom = RegisterClassW(&windowClass);
    
    HWND window = CreateWindowW(windowClass.lpszClassName, 
                                L"Pong", 
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE, 
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                WIDTH, HEIGHT, 
                                NULL, NULL, instance, NULL);
    
    ShowWindow(window, cmdShow);
    
    for(;;) {
        MSG msg;
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }    
    }

    return 0;
}

LRESULT CALLBACK WinProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch(msg) {
        case WM_KEYDOWN: {
            switch(wParam) {
                case 'O': { 
                    DestroyWindow(window); 
                } break;
            }
        } break;
        
        case WM_DESTROY: {
            PostQuitMessage(0);
        } break;
        
        default: {
            return DefWindowProcW(window, msg, wParam, lParam);
        }
    }
    
    return 0;
}
