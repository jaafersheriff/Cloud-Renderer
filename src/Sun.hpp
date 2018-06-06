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
            spatial.scale = glm::vec3(outerRadius);

            glm::vec3 min = glm::vec3(vol->xBounds.x, vol->yBounds.x, vol->zBounds.x);
            glm::vec3 max = glm::vec3(vol->xBounds.y, vol->yBounds.y, vol->zBounds.y);

            glm::vec3 lookDir = glm::normalize(vol->position - spatial.position);
            float minLen = glm::length(min);
            float maxLen = glm::length(max);
            glm::vec3 lookPos = vol->position - lookDir * glm::max(minLen, maxLen);
            V = glm::lookAt(lookPos, vol->position, glm::vec3(0, 1, 0));

            // TODO : these are all working fail-safes
            // TODO : more math here to have an accurate, tight viewport
            float minmin = glm::min(min.x, glm::min(min.y, min.z));
            float maxmax = glm::max(max.x, glm::max(max.y, max.z));
            float nearPlane = 0.01f;
            float farPlane = 2.f * glm::max(minLen, maxLen);
            P = glm::ortho(minmin, maxmax, minmin, maxmax, nearPlane, farPlane);
        }
};

#endif
