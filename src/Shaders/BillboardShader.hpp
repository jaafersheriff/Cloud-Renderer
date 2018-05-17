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
        BillboardShader(std::string, std::string);

        /* Render billboard list */
        void render(std::vector<Spatial *> &, Texture *, Texture *);
};

#endif