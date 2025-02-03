#ifndef WINDOW_HANDLER_HPP
#define WINDOW_HANDLER_HPP

#include <Windows.h>

#include "InputHandler.hpp"

class Window {
  private:
    HWND hWnd;
    UINT width;
    UINT height;
    HINSTANCE instance;

    static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

  public:
    Window(const HINSTANCE instance, const UINT width, const UINT height, int nCmdShow);
    ~Window();

    InputHandler inputHandler;
    // inline void resetKeyPressAndRelease() { this->inputHandler.clearKeyStates(); }
    HWND GetHWND() const;
    UINT GetWidth() const;
    UINT GetHeight() const;
    void Show(int nCmdShow) const;
};

#endif