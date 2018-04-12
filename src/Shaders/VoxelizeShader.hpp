#pragma once
#ifndef _VOXELIZE_SHADER_HPP_
#define _VOXELIZE_SHADER_HPP_

#include "Shader.hpp"

#include "Model/Texture.hpp"
#include "Volume.hpp"

class VoxelizeShader : public Shader {
    public:
        VoxelizeShader(std::string vertex, std::string fragment) :
            Shader(vertex, fragment)
        {}
        enum Stage {
            None,       // 0
            Voxelize,   // 1
            Positions,  // 2 
            ConeTrace   // 3
        };

        bool init(Volume *, int, int);

        /* Generate 3D volume */
        void voxelize();
        void renderQuad(glm::mat4, glm::mat4, glm::vec3, VoxelizeShader::Stage);

        /* 2D position texture */
        Texture * positionMap;
        void clearPositionMap();

        Volume * volume;

        // TODO : calculate this to maximize performance
        float steps = 0.2f;
    private:

        void bindVolume();
        void unbindVolume();

        void initPositionMap(int, int);
};

#endif