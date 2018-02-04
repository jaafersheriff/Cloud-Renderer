#include "Camera.hpp"

#include "Window.hpp"
#include "glm/gtc/matrix_transform.hpp"

Camera::Camera(const glm::vec3 position) {
    /* Init */
    this->position = position;
    phi = theta = 0.0;

    update();
}

/* Update look at point
 * dx and dy correspond to mouse movement */
void Camera::takeMouseInput(const double dx, const double dy) {
    theta += dx * LOOK_SPEED;
    phi -= dy * LOOK_SPEED;
}

void Camera::update() {
    updateUVW();
    updateLookAt();
    this->P = glm::perspective(45.f, (float) (Window::width / Window::height), 0.01f, 2500.f);
    this->V = glm::lookAt(position, lookAt, glm::vec3(0, 1, 0));
}

void Camera::updateUVW() {
    w = glm::normalize(lookAt - position);
    u = glm::normalize(glm::cross(w, glm::vec3(0, 1, 0)));
    v = glm::normalize(glm::cross(u, w));
}

void Camera::updateLookAt() {
    glm::vec3 sphere(
            glm::cos(phi)*glm::cos(theta),
            glm::sin(phi),
            glm::cos(phi)*glm::cos((3.1415f/2.f)-theta));
    lookAt = position + glm::normalize(sphere);
}

/* All movement is based on UVW basis-vectors */
void Camera::moveForward(const float timeStep) { 
    position += w * MOVE_SPEED * timeStep;
    lookAt += w * MOVE_SPEED * timeStep;
}

void Camera::moveBackward(const float timeStep) { 
    position -= w * MOVE_SPEED * timeStep;
    lookAt -= w * MOVE_SPEED * timeStep;
}

void Camera::moveRight(const float timeStep) { 
    position += u * MOVE_SPEED * timeStep;
    lookAt += u * MOVE_SPEED * timeStep;
}

void Camera::moveLeft(const float timeStep) { 
    position -= u * MOVE_SPEED * timeStep;
    lookAt -= u * MOVE_SPEED * timeStep;
}

void Camera::moveUp(const float timeStep) { 
    position += v * MOVE_SPEED * timeStep;
    lookAt += v * MOVE_SPEED * timeStep;
}

void Camera::moveDown(const float timeStep) { 
    position -= v * MOVE_SPEED * timeStep;
    lookAt -= v * MOVE_SPEED * timeStep;
}
