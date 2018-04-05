#pragma once
#ifndef _LIGHT_DEPTH_SHADER_HPP_
#define _LIGHT_DEPTH_SHADER_HPP_

#include "Shader.hpp"
#include "Model/Mesh.hpp"
#include "Model/Texture.hpp"
#include "Spatial.hpp"
#include "VolumeShader.hpp"

class LightDepthShader : public Shader {
    public:
        LightDepthShader(const std::string vert, const std::string frag);

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