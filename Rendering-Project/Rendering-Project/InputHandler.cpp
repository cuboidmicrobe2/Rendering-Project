#include "InputHandler.hpp"
#include <d3d11.h>
#include <windowsx.h>

bool InputHandler::handleInputMessage(const UINT message, const WPARAM wParam, const LPARAM lParam) {
    switch (message) {
        case WM_KEYDOWN: {
            const unsigned char key      = static_cast<unsigned char>(wParam);
            const bool wasPreviouslyDown = lParam & (1 << 30);
            if (!wasPreviouslyDown) this->setKeyState(key, InputHandler::DOWN | InputHandler::PRESSED);
            return true;
        }
        case WM_KEYUP: {
            const unsigned char key = static_cast<unsigned char>(wParam);
            this->setKeyState(key, InputHandler::RELEASED);
            return true;
        }
        // Mouse Input
        case WM_MOUSEMOVE: {
            const int xPos = GET_X_LPARAM(lParam);
            const int yPos = GET_Y_LPARAM(lParam);
            this->setMousePos(xPos, yPos);
            return true;
        }
        case WM_LBUTTONDOWN: {
            if (!this->LMDowm()) this->setLMouseKeyState(InputHandler::DOWN | InputHandler::PRESSED);
            return true;
        }
        case WM_LBUTTONUP: {
            this->setLMouseKeyState(InputHandler::RELEASED);
            return true;
        }
        case WM_RBUTTONDOWN: {
            if (!this->RMDowm()) this->setRMouseKeyState(InputHandler::DOWN | InputHandler::PRESSED);
            return true;
        }
        case WM_RBUTTONUP: {
            this->setRMouseKeyState(InputHandler::RELEASED);
            return true;
        }
        default:
            return false;
    }
    return true;
}