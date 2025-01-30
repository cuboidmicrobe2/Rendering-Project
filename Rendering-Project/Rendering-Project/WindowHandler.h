#ifndef WINDOW_HANDLER_HPP
#define WINDOW_HANDLER_HPP

#include <Windows.h>

class Window {
private:
	HWND window;
	UINT width;
	UINT height;
    HINSTANCE instance;

	static LRESULT CALLBACK StaticWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
	Window(const HINSTANCE instance, const UINT width, const UINT height, int nCmdShow);
	~Window();

	void Show(int nCmdShow) const;
};

#endif