#pragma once
#ifndef _BILLBOARD_SHADER_HPP_
#define _BILLBOARD_SHADER_HPP_

#include "Shader.hpp"
#include "Model/Mesh.hpp"
#include "Model/Texture.hpp"
#include "Spatial.hpp"

#include "Light.hpp"

class BillboardShader : public Shader {
    public:
        BillboardShader(std::string vertex, std::string fragment) :
            Shader(vertex, fragment)
        {}

        bool init();

        /* Render billboard list */
        void render(std::vector<Spatial *> &);
};

#endif