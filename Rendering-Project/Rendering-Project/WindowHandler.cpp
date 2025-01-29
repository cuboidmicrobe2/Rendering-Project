#include "WindowHandler.h"
#include <exception>
#include <iostream>
#include <string>

LRESULT Window::_WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) { 
    switch (message) {
    case WM_DESTROY: {
        PostQuitMessage(0);
        return 0;
    }
    default:
        break;
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

Window::Window(const HINSTANCE instance, const UINT width, const UINT height, int nCmdShow)
    : width(width), height(height) {

    const wchar_t CLASS_NAME[] = L"WINDOW CLASS";

    WNDCLASS wc = {.lpfnWndProc = this->_WindowProc, .hInstance = instance, .lpszClassName = CLASS_NAME};

    RegisterClass(&wc);

    this->window = CreateWindowEx(0, CLASS_NAME, L"Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, this->width,
                                  this->height, nullptr, nullptr, instance, nullptr);

    if (window == nullptr) {
        throw std::runtime_error("window bad, last error: " + std::to_string(GetLastError()));
    }

    ShowWindow(this->window, nCmdShow);
}

Window::~Window() {}

LRESULT Window::WindowProc() { return LRESULT(); }
