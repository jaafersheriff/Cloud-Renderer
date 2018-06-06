#include "Camera.hpp"

#include "IO/Window.hpp"
#include "Util.hpp"

#include "glm/gtc/matrix_transform.hpp"

double Camera::phi, Camera::theta;
glm::mat4 Camera::P, Camera::V;
glm::vec3 Camera::position, Camera::lookAt, Camera::u, Camera::v, Camera::w;

void Camera::update() {
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

    /* Get move speed */
    float moveSpeed = MOVE_SPEED;
    if (Keyboard::isKeyPressed(GLFW_KEY_LEFT_SHIFT) || Keyboard::isKeyPressed(GLFW_KEY_RIGHT_SHIFT)) {
        moveSpeed *= 2.f;
    }

    /* Update position*/
    if (Keyboard::isKeyPressed(GLFW_KEY_W)) {
        moveForward(moveSpeed);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_S)) {
        moveBackward(moveSpeed);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_A)) {
        moveLeft(moveSpeed);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_D)) {
        moveRight(moveSpeed);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_R)) {
        moveUp(moveSpeed);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_E)) {
        moveDown(moveSpeed);
    }   

    /* Update matrices */
    // TODO : only update if necessary
    P = glm::perspective(45.f, (float) (Window::width / Window::height), 0.01f, 2500.f);
    V = glm::lookAt(position, lookAt, glm::vec3(0, 1, 0));
}

/* All movement is based on UVW basis-vectors */
void Camera::moveForward(const float moveSpeed) { 
    position += w * moveSpeed;
    lookAt += w * moveSpeed;
}

void Camera::moveBackward(const float moveSpeed) { 
    position -= w * moveSpeed;
    lookAt -= w * moveSpeed;
}

void Camera::moveRight(const float moveSpeed) { 
    position += u * moveSpeed;
    lookAt += u * moveSpeed;
}

void Camera::moveLeft(const float moveSpeed) { 
    position -= u * moveSpeed;
    lookAt -= u * moveSpeed;
}

void Camera::moveUp(const float moveSpeed) { 
    position += v * moveSpeed;
    lookAt += v * moveSpeed;
}

void Camera::moveDown(const float moveSpeed) { 
    position -= v * moveSpeed;
    lookAt -= v * moveSpeed;
}
