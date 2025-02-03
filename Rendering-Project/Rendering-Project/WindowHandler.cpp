#include "WindowHandler.hpp"
#include <exception>
#include <iostream>
#include <string>
#include <windowsx.h>

LRESULT Window::StaticWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_NCCREATE) {
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        Window* window        = reinterpret_cast<Window*>(pCreate->lpCreateParams);
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
        window->hWnd = hWnd;
        return window->WindowProc(hWnd, message, wParam, lParam);
    }

    Window* window = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    if (window) {
        return window->WindowProc(hWnd, message, wParam, lParam);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

LRESULT Window::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (this->inputHandler.handleInputMessage(message, wParam, lParam)) return 0;

    switch (message) {

        // On X
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

Window::Window(const HINSTANCE instance, const UINT width, const UINT height, int nCmdShow)
    : instance(instance), width(width), height(height), hWnd(nullptr) {

    const wchar_t CLASS_NAME[] = L"WINDOW_CLASS";

    WNDCLASSEX wc = {.cbSize        = sizeof(WNDCLASSEX),
                     .lpfnWndProc   = Window::StaticWindowProc,
                     .hInstance     = this->instance,
                     .lpszClassName = CLASS_NAME};

    if (!RegisterClassEx(&wc)) {
        throw std::runtime_error("Failed to register window class, error: " + std::to_string(GetLastError()));
    }

    this->hWnd = CreateWindowEx(0, CLASS_NAME, L"Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, this->width,
                                this->height, nullptr, nullptr, this->instance, this);

    if (!this->hWnd) {
        throw std::runtime_error("Failed to create window, error: " + std::to_string(GetLastError()));
    }

    this->Show(nCmdShow);
}

Window::~Window() {
    if (this->hWnd) {
        DestroyWindow(this->hWnd);
    }
    UnregisterClass(L"WINDOW_CLASS", this->instance);
}

HWND Window::GetHWND() const { return this->hWnd; }

UINT Window::GetWidth() const { return this->width; }

UINT Window::GetHeight() const { return this->height; }

void Window::Show(int nCmdShow) const {
    ShowWindow(this->hWnd, nCmdShow);
    UpdateWindow(this->hWnd);
}