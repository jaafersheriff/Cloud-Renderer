#pragma once
#ifndef _CLOUD_HPP_
#define _CLOUD_HPP_

#include "Model/Texture.hpp"

class Cloud {
    public:
        Cloud() {}

        glm::vec3 position = glm::vec3(0.f);
        glm::vec2 size = glm::vec2(1.f);
        float rotation = 0.f;
};

#endif