
#include "Window.hpp"
#include "Camera.hpp"

Window window;
Camera camera;

void takeInput() {
    if (Mouse::isDown(0)) {
        camera.takeMouseInput(Mouse::dx, Mouse::dy);
    }
    else {
        camera.takeMouseInput(0.0, 0.0);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_W)) {
        camera.moveForward(1.f);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_S)) {
        camera.moveBackward(1.f);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_A)) {
        camera.moveLeft(1.f);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_D)) {
        camera.moveRight(1.f);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_E)) {
        camera.moveUp(1.f);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_R)) {
        camera.moveDown(1.f);
    }   
}

int main() {
    window.init("Clouds");
    

    while (!window.shouldClose()) {
        window.update();
        takeInput();
        camera.update();
    }
}
