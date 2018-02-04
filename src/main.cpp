
#include "Window.hpp"
#include "Camera.hpp"

Window window;
Camera camera;

/* Timing */
double timeStep, lastFpsTime, lastFrameTime, runTime;
int nFrames, FPS;

void updateTiming() {
    runTime = (float)window.getTime();
    timeStep = runTime - lastFrameTime;
    lastFrameTime = runTime;
    nFrames++;
    if (runTime - lastFpsTime >= 1.0) {
        FPS = nFrames;
        nFrames = 0;
        lastFpsTime = runTime;
    }
}

void takeInput() {
    if (Mouse::isDown(0)) {
        camera.takeMouseInput(Mouse::dx, Mouse::dy);
    }
    else {
        camera.takeMouseInput(0.0, 0.0);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_W)) {
        camera.moveForward(timeStep);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_S)) {
        camera.moveBackward(timeStep);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_A)) {
        camera.moveLeft(timeStep);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_D)) {
        camera.moveRight(timeStep);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_E)) {
        camera.moveUp(timeStep);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_R)) {
        camera.moveDown(timeStep);
    }   
}

int main() {
    /* Timing */
    timeStep = lastFpsTime = lastFrameTime = runTime = 0.0;
    nFrames = FPS = 0;

    window.init("Clouds");
    
    while (!window.shouldClose()) {
        updateTiming();
        window.update();
        takeInput();
        camera.update();
    }
}
