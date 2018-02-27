#include "Camera.hpp"

#include "IO/Window.hpp"
#include "Util.hpp"

#include "glm/gtc/matrix_transform.hpp"

double Camera::phi, Camera::theta;
glm::mat4 Camera::P, Camera::V;
glm::vec3 Camera::position, Camera::lookAt, Camera::u, Camera::v, Camera::w;

Camera::Camera(const glm::vec3 position) {
    /* Init */
    this->position = position;
    phi = theta = 0.0;

    update(0.f);
}

void Camera::update(float dt) {
    /* Update view angles */
    if (Mouse::isDown(0)) {
        theta += Mouse::dx * LOOK_SPEED;
        phi -= Mouse::dy * LOOK_SPEED;
    }

    /* Upate view basis vectors */
    w = glm::normalize(lookAt - position);
    u = glm::normalize(glm::cross(w, glm::vec3(0, 1, 0)));
    v = glm::normalize(glm::cross(u, w));

    /* Update look at */
    glm::vec3 sphere(
            glm::cos(phi)*glm::cos(theta),
            glm::sin(phi),
            glm::cos(phi)*glm::cos((Util::PI/2.f)-theta));
    lookAt = position + glm::normalize(sphere);

    /* Update position*/
    if (Keyboard::isKeyPressed(GLFW_KEY_W)) {
        moveForward(dt);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_S)) {
        moveBackward(dt);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_A)) {
        moveLeft(dt);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_D)) {
        moveRight(dt);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_R)) {
        moveUp(dt);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_E)) {
        moveDown(dt);
    }   

    /* Update matrices */
    // TODO : only update if necessary
    P = glm::perspective(45.f, (float) (Window::width / Window::height), 0.01f, 2500.f);
    V = glm::lookAt(position, lookAt, glm::vec3(0, 1, 0));
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
