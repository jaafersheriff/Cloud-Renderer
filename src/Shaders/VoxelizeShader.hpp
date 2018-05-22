#pragma once
#ifndef _VOXELIZE_SHADER_HPP_
#define _VOXELIZE_SHADER_HPP_

#include "Shader.hpp"

#include "Model/Texture.hpp"
#include "Volume.hpp"

class VoxelizeShader {
    public:
        VoxelizeShader(std::string, std::string, std::string);

        Shader * firstVoxelizer;
        Shader * secondVoxelizer;

        /* Generate 3D volume */
        void voxelize(Volume *);

        /* 2D position FBO */
        GLuint positionFBO;
        Texture * positionTexture;

    private:
        void firstVoxelize(Volume *);
        void secondVoxelize(Volume *);
        
        void bindVolume(Shader *, Volume *);
        void unbindVolume();
};

#endif