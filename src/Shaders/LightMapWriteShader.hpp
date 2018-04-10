#pragma once
#ifndef _LIGHT_MAP_WRITE_SHADER_HPP_
#define _LIGHT_MAP_WRITE_SHADER_HPP_

#include "Shader.hpp"
#include "Model/Mesh.hpp"
#include "Model/Texture.hpp"
#include "Spatial.hpp"
#include "Volume.hpp"

class LightMapWriteShader : public Shader {
    public:
        LightMapWriteShader(const std::string vert, const std::string frag);

        bool init(int, int);

        void render(std::vector<Volume::Voxel> &);

        Texture * lightMap;
    private:
        void initFBO();
        GLuint fboHandle;

        glm::mat4 L;
};

#endif