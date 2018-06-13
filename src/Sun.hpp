#pragma once
#ifndef _SUN_HPP_
#define _SUN_HPP_

#include "IO/Window.hpp"
#include "CloudVolume.hpp"

#include "glm/gtc/matrix_transform.hpp"

class Sun {
    public: 
        static glm::vec3 position;

        static glm::mat4 P;
        static glm::mat4 V;

        static glm::vec3 nearPlane;
        static glm::vec3 farPlane;
        static float clipDistance;

        static glm::vec3 innerColor;
        static glm::vec3 outerColor;
        static float innerRadius;
        static float outerRadius;

        static void update(CloudVolume *vol) {
            glm::vec3 min = glm::vec3(vol->xBounds.x, vol->yBounds.x, vol->zBounds.x);
            glm::vec3 max = glm::vec3(vol->xBounds.y, vol->yBounds.y, vol->zBounds.y);

            glm::vec3 lookDir = glm::normalize(vol->position - position);
            float minLen = glm::length(min);
            float maxLen = glm::length(max);
            glm::vec3 lookPos = vol->position - lookDir * glm::max(minLen, maxLen);
            V = glm::lookAt(lookPos, vol->position, glm::vec3(0, 1, 0));

            float minmin = 2.f * glm::min(min.x, glm::min(min.y, min.z));
            float maxmax = 2.f * glm::max(max.x, glm::max(max.y, max.z));

            nearPlane = lookPos + lookDir * 0.01f;
            farPlane = lookPos + lookDir * 2.f * glm::max(minLen, maxLen);
            clipDistance = glm::distance(nearPlane, farPlane);
            P = glm::ortho(minmin, maxmax, minmin, maxmax, 0.01f, 0.01f + clipDistance);
        }
};

#endif
