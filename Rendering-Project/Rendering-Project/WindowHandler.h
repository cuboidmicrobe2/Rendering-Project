#ifndef WINDOW_HANDLER_HPP
#define WINDOW_HANDLER_HPP

#include <Windows.h>

class Window {
private:
	HWND window;
	UINT width;
	UINT height;

	static LRESULT CALLBACK _WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

public:
	Window(const HINSTANCE instance, const UINT width, const UINT height, int nCmdShow);
	~Window();

	LRESULT CALLBACK WindowProc();
};

#endif