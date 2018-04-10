#pragma once
#ifndef _VOLUME_SHADER_HPP_
#define _VOLUME_SHADER_HPP_

#include "Shader.hpp"

#include "Model/Mesh.hpp"
#include "Volume.hpp"

class VolumeShader : public Shader {
    public:
        VolumeShader(std::string vertex, std::string fragment) :
            Shader(vertex, fragment)
        {}

        bool init(Volume * volume);

        /* Generate 3D volume */
        void voxelize(glm::mat4, glm::mat4, glm::vec3, GLuint);
        void renderMesh(glm::mat4, glm::mat4, glm::vec3, bool, GLuint);

        float normalStep = 0.2f;
        float visibilityContrib = 0.02f;

        Volume * volume;
};

#endif