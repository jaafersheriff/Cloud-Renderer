#pragma once
#ifndef _VOXELIZE_SHADER_HPP_
#define _VOXELIZE_SHADER_HPP_

#include "Shader.hpp"

#include "Model/Mesh.hpp"
#include "Model/Texture.hpp"
#include "Volume.hpp"

class VoxelizeShader : public Shader {
    public:
        VoxelizeShader(std::string vertex, std::string fragment) :
            Shader(vertex, fragment)
        {}

        bool init(Volume *, int, int);

        /* Generate 3D volume */
        void voxelize(glm::mat4, glm::mat4, glm::vec3);
        void renderMesh(glm::mat4, glm::mat4, glm::vec3, bool);

        float normalStep = 0.2f;
        float visibilityContrib = 0.02f;

        Volume * volume;

        void clearPositionMap();
        Texture * positionMap;
    private:
        void initFBO();
        GLuint positionMapFBOId;
};

#endif