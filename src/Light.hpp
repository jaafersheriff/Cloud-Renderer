#pragma once
#ifndef _LIGHT_HPP_
#define _LIGHT_HPP_

#include "Spatial.hpp"

#include "glm/gtc/matrix_transform.hpp"

class Light {
    public:
        static Spatial spatial;
        static glm::mat4 P;
        static glm::mat4 V;

        static float nearPlane;
        static float farPlane;

        static void update(glm::vec3 lookAt, glm::vec2 xBounds, glm::vec2 yBounds) {
            P = glm::ortho(xBounds.x, xBounds.y, yBounds.x, yBounds.y, nearPlane, farPlane);
            V = glm::lookAt(spatial.position, lookAt, glm::vec3(0, 1, 0));
        }
};

#endif
