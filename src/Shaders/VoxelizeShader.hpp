#pragma once
#ifndef _VOXELIZE_SHADER_HPP_
#define _VOXELIZE_SHADER_HPP_

#include "Shader.hpp"

#include "Model/Texture.hpp"
#include "Volume.hpp"

class VoxelizeShader : public Shader {
    public:
        VoxelizeShader(std::string, std::string);
            
        enum Stage {
            None,       // 0
            Voxelize,   // 1
            Positions,  // 2 
        };

        /* Generic function to render quad -- takes Stage as a parameter to orchestrate GL binds */
        void renderQuad(Volume *, glm::mat4, glm::mat4, glm::vec3, VoxelizeShader::Stage);

        /* Generate 3D volume */
        void voxelize(Volume *);

        /* 2D position texture */
        Texture * positionMap;
        void clearPositionMap();

    private:
        void bindVolume(Volume *);
        void unbindVolume();

        void initPositionMap(int, int);
};

#endif