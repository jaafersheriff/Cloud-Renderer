#include "Mouse.hpp"

double Mouse::x = 0.0;
double Mouse::y = 0.0;
double Mouse::dx = 0.0;
double Mouse::dy = 0.0;
int Mouse::mouseButtons[GLFW_MOUSE_BUTTON_LAST] = { GLFW_RELEASE };

void Mouse::update(double newX, double newY) {
    /* Calculate x-y speed */
    dx = newX - x;
    dy = newY - y;

    /* Set new positions */
    // TODO: if newX > 0 and newY > 0
    x = newX;
    y = newY;

    // TODO : dw = scroll whell
}

bool Mouse::isDown(int button) {
    return mouseButtons[button] >= GLFW_PRESS;
}

bool Mouse::isUp(int button) {
    return mouseButtons[button] == GLFW_RELEASE;
}

void Mouse::setButtonStatus(int button, int action) {
    mouseButtons[button] = action;
}
