#pragma once
#ifndef _BILLBOARD_SHADER_HPP_
#define _BILLBOARD_SHADER_HPP_

#include "Shader.hpp"
#include "Model/Mesh.hpp"
#include "Model/Texture.hpp"

class Cloud;
class BillboardShader : public Shader {
    public:
        BillboardShader(std::string vertex, std::string fragment) :
            Shader(vertex, fragment)
        {}

        bool init(std::string, std::string, Mesh *);

        /* Render billboard list */
        void render(glm::vec3);

        /* Add billboard to list at a given position */
        void addCloud(glm::vec3 pos);

        Mesh *quad;
    private:
        /* List of render target */
        std::vector<Cloud *> clouds;

        /* Assets */
        Texture *diffuseTex;
        Texture *normalTex;
};

#endif