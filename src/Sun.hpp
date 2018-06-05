#pragma once
#ifndef _SUN_HPP_
#define _SUN_HPP_

#include "Spatial.hpp"
#include "IO/Window.hpp"
#include "CloudVolume.hpp"

#include "glm/gtc/matrix_transform.hpp"

class Sun {
    public: 
        static Spatial spatial;
        static glm::mat4 P;
        static glm::mat4 V;

        static glm::vec3 innerColor;
        static glm::vec3 outerColor;
        static float innerRadius;
        static float outerRadius;

        static void update(CloudVolume *vol) {
            P = glm::perspective(45.f, (float)Window::width / Window::height, 0.1f, 1000.f);

            glm::vec3 min = glm::vec3(vol->xBounds.x, vol->yBounds.x, vol->zBounds.x);
            glm::vec3 max = glm::vec3(vol->xBounds.y, vol->yBounds.y, vol->zBounds.y);
            glm::vec3 lookDir = glm::normalize(spatial.position - vol->position);
            // TODO : if perspective doesn't work out, scale this by 1.1f
            glm::vec3 lookPos = vol->position + lookDir * glm::max(glm::length(min), glm::length(max));
            V = glm::lookAt(lookPos, vol->position, glm::vec3(0, 1, 0));
            spatial.scale = glm::vec3(outerRadius);
        }
};

#endif
