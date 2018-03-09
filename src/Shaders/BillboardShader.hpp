#pragma once
#ifndef _BILLBOARD_SHADER_HPP_
#define _BILLBOARD_SHADER_HPP_

#include "Shader.hpp"
#include "Model/Mesh.hpp"
#include "Model/Texture.hpp"
#include "Spatial.hpp"

class BillboardShader : public Shader {
    public:
        BillboardShader(std::string vertex, std::string fragment) :
            Shader(vertex, fragment)
        {}

        bool init(std::string, std::string, Mesh *);

        /* Render billboard list */
        void render(glm::vec3, std::vector<Spatial *> &);

    private:
        Mesh *quad;
        Texture *diffuseTex;
        Texture *normalTex;
        glm::vec2 texSize;
};

#endif