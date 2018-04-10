#pragma once
#ifndef _SPATIAL_HPP_
#define _SPATIAL_HPP_

#include "glm/glm.hpp"

class Spatial {
    public:
        Spatial() :
            position(0.f),
            scale(0.f),
            rotation(0.f)
        {}

        Spatial(glm::vec3 p, glm::vec3 s, glm::vec3 r) :
            position(p),
            scale(s),
            rotation(r)
        {}

        glm::vec3 position;
        glm::vec3 scale;
        glm::vec3 rotation;
};

#endif