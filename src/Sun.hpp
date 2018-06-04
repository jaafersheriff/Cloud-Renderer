#pragma once
#ifndef _SUN_HPP_
#define _SUN_HPP_

#include "Spatial.hpp"

#include "glm/gtc/matrix_transform.hpp"

class Sun {
    public:
        static Spatial spatial;
        static glm::mat4 V;

        static glm::vec3 innerColor;
        static glm::vec3 outerColor;
        static float innerRadius;
        static float outerRadius;

        static void update(glm::vec3 lookAt) {
            V = glm::lookAt(spatial.position, lookAt, glm::vec3(0, 1, 0));
            spatial.scale = glm::vec3(outerRadius);
        }
};

#endif
