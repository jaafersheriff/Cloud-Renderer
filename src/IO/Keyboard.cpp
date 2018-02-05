#include "Keyboard.hpp"

#include "GLFW/glfw3.h"

bool Keyboard::keyStatus[NUM_KEYS] = { false };

bool Keyboard::isKeyPressed(int key) {
    return keyStatus[key] == GLFW_PRESS;
}

void Keyboard::setKeyStatus(int key, int action) {
    keyStatus[key] = (action == GLFW_PRESS);
}
