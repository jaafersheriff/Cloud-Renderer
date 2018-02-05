#pragma once
#ifndef _CLOUD_HPP_
#define _CLOUD_HPP_

#include "Model/Mesh.hpp";
#include "Model/Texture.hpp"

class Cloud {
    public:
        Cloud() {
            mesh = new Mesh;
            mesh->vertBuf = {
                -1.f, -1.f,  1.f,
                 1.f, -1.f,  1.f,
                 1.f,  1.f,  1.f,
                -1.f,  1.f,  1.f
            };
            mesh->eleBuf = {
                0, 1, 2,
                2, 3, 0
            };
            mesh->texBuf = {
                0.f, 0.f,
                1.f, 0.f,
                1.f, 1.f, 
                0.f, 1.f
            };

            mesh->init();
        }

        Mesh *mesh = nullptr;
        Texture *texture = nullptr;
        Texture *normalMap = nullptr;

        glm::vec3 position = glm::vec3(0.f);
        glm::vec3 rotation = glm::vec3(0.f);
        glm::vec3 scale = glm::vec3(0.f);
};

#endif