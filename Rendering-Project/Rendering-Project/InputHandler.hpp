#ifndef INPUT_HANDLER_HPP
#define INPUT_HANDLER_HPP

#include <array>
#include <iostream>

class InputHandler {
  public:
    InputHandler() : keyStates({}), mousePos({}), mouseMovement({}), LMouseButton(0), RMouseButton(0) {};

    enum KeyStateMasks {
        DOWN = 1,

        PRESSED = 2,
        RELEASED = 4,
    };

    inline void setKeyState(const unsigned char key, const unsigned char state) { keyStates[key] = state; }
    inline void setLMouseKeyState(const unsigned char state) { this->LMouseButton = state; }
    inline void setRMouseKeyState(const unsigned char state) { this->RMouseButton = state; }

    inline void setMousePos(const unsigned int x, const unsigned int y) {
        this->mouseMovement = {(int)x - this->mousePos.first, (int)y - this->mousePos.second};
        this->mousePos = {x, y};
    }

    void reset() {
        for (unsigned char& keyState : keyStates)
            keyState &= DOWN;
        this->mouseMovement = {0, 0};
        this->LMouseButton &= DOWN;
        this->RMouseButton &= DOWN;
    }

    inline void clearKeyState(const unsigned char key) { keyStates[key] &= DOWN; }

    inline bool isDown(const unsigned char key) const { return keyStates[key] & DOWN; }
    inline bool wasPressed(const unsigned char key) const { return keyStates[key] & PRESSED; }
    inline bool wasReleased(const unsigned char key) const { return keyStates[key] & RELEASED; }

    inline bool LMDowm() const { return this->LMouseButton & DOWN; }
    inline bool LMPressed() const { return this->LMouseButton & PRESSED; }
    inline bool LMReleased() const { return this->LMouseButton & RELEASED; }

    inline bool RMDowm() const { return this->RMouseButton & DOWN; }
    inline bool RMPressed() const { return this->RMouseButton & PRESSED; }
    inline bool RMReleased() const { return this->RMouseButton & RELEASED; }

    inline std::pair<int, int> getMouseMovement() const { return this->mouseMovement; };
    inline std::pair<unsigned int, unsigned int> getMousePos() const { return this->mousePos; };

  private:
    std::pair<unsigned int, unsigned int> mousePos;
    std::pair<int, int> mouseMovement;
    unsigned char LMouseButton;
    unsigned char RMouseButton;

    std::array<unsigned char, 256> keyStates;
};

#endif