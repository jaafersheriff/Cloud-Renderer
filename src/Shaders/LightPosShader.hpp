#pragma once
#ifndef _LIGHT_POS_SHADER_HPP_
#define _LIGHT_POS_SHADER_HPP_

#include "Shader.hpp"
#include "Model/Mesh.hpp"
#include "Model/Texture.hpp"
#include "Spatial.hpp"
#include "VolumeShader.hpp"

class LightPosShader : public Shader {
    public:
        LightPosShader(const std::string vert, const std::string frag);

        bool init();

        void render(Mesh *, std::vector<VolumeShader::Voxel> &);

        Texture * lightMap;
        void setTextureSize(int);
    private:
        void initFBO();
        GLuint fboHandle;

        glm::mat4 L;
};

#endif