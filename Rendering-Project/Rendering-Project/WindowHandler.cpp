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
    switch (message) {

            // Keyboard Input
        case WM_KEYDOWN: {
            const unsigned char key      = static_cast<unsigned char>(wParam);
            const bool wasPreviouslyDown = lParam & (1 << 30);
            if (!wasPreviouslyDown) {
                this->inputHandler.setKeyState(key, InputHandler::DOWN | InputHandler::PRESSED);
            }
            if (key == VK_F11) { // Toggle fullscreen on F11 key press
                this->isFullscreen = !this->isFullscreen;
                this->SetFullscreen(this->isFullscreen);
            }
            if (key == VK_ESCAPE) { // Quit on ESC key press
                PostQuitMessage(0);
                return 0;
            }
            return 0;
        }
        case WM_KEYUP: {
            const unsigned char key = static_cast<unsigned char>(wParam);
            this->inputHandler.setKeyState(key, InputHandler::RELEASED);
            return 0;
        }
        // Mouse Input
        case WM_MOUSEMOVE: {
            const int xPos = GET_X_LPARAM(lParam);
            const int yPos = GET_Y_LPARAM(lParam);
            this->inputHandler.setMousePos(xPos, yPos);
            return 0;
        }
        case WM_LBUTTONDOWN: {
            if (!this->inputHandler.LMDowm())
                this->inputHandler.setLMouseKeyState(InputHandler::DOWN | InputHandler::PRESSED);
            return 0;
        }
        case WM_LBUTTONUP: {
            this->inputHandler.setLMouseKeyState(InputHandler::RELEASED);
            return 0;
        }
        case WM_RBUTTONDOWN: {
            if (!this->inputHandler.RMDowm())
                this->inputHandler.setRMouseKeyState(InputHandler::DOWN | InputHandler::PRESSED);
            return 0;
        }
        case WM_RBUTTONUP: {
            this->inputHandler.setRMouseKeyState(InputHandler::RELEASED);
            return 0;
        }

        // On X
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

Window::Window(const HINSTANCE instance, int nCmdShow, const UINT width, const UINT height)
    : instance(instance), width(width), height(height), hWnd(nullptr), isFullscreen(false) {

    const wchar_t CLASS_NAME[] = L"WINDOW_CLASS";

    WNDCLASSEX wc = {.cbSize        = sizeof(WNDCLASSEX),
                     .lpfnWndProc   = Window::StaticWindowProc,
                     .hInstance     = this->instance,
                     .lpszClassName = CLASS_NAME};

    if (!RegisterClassEx(&wc)) {
        throw std::runtime_error("Failed to register window class, error: " + std::to_string(GetLastError()));
    }

    bool needsExtraHelp = false;
    if (width == 0 && height == 0) {
        needsExtraHelp          = true;
        MONITORINFO monitorInfo = {sizeof(monitorInfo)};
        if (GetMonitorInfo(MonitorFromWindow(GetDesktopWindow(), MONITOR_DEFAULTTOPRIMARY), &monitorInfo)) {
            this->width  = monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left;
            this->height = monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top;
        }
    }

    this->hWnd = CreateWindowEx(0, CLASS_NAME, L"Window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, 0, this->width,
                                this->height, nullptr, nullptr, this->instance, this);

    if (!this->hWnd) {
        throw std::runtime_error("Failed to create window, error: " + std::to_string(GetLastError()));
    }

    if (needsExtraHelp) {
        this->isFullscreen = !this->isFullscreen;
        this->SetFullscreen(this->isFullscreen);
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

void Window::SetFullscreen(bool fullscreen) const {
    if (fullscreen) {
        SetWindowLongPtr(this->hWnd, GWL_STYLE, WS_POPUP | WS_VISIBLE);
        MONITORINFO mi = {sizeof(mi)};
        if (GetMonitorInfo(MonitorFromWindow(this->hWnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowPos(this->hWnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLongPtr(this->hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW | WS_VISIBLE);
        SetWindowPos(this->hWnd, HWND_TOP, 0, 0, this->width, this->height, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}