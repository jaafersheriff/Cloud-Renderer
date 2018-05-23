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
        BillboardShader(const std::string &r, const std::string &v, const std::string &f) :
            Shader(r, v, f) 
        {}

        /* Render billboard list */
        void render(std::vector<Spatial *> &, Texture *, Texture *);
};

#endif