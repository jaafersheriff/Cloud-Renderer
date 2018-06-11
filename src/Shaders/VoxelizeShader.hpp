#pragma once
#ifndef _VOXELIZE_SHADER_HPP_
#define _VOXELIZE_SHADER_HPP_

#include "Shader.hpp"

#include "Model/Texture.hpp"
#include "CloudVolume.hpp"

class VoxelizeShader {
    public:
        VoxelizeShader(const std::string &, const std::string &, const std::string &, const std::string &, const std::string &);

        Shader * firstVoxelizer;
        Shader * secondVoxelizer;

        /* Generate 3D volume */
        void voxelize(CloudVolume *);

        /* 2D position FBO */
        GLuint positionFBO;
        Texture * positionMap;
        Texture * depthMap;
        void clearPositionMap();

    private:
        void firstVoxelize(CloudVolume *);
        void secondVoxelize(CloudVolume *);
        
        void bindVolume(Shader *, CloudVolume *);
        void unbindVolume();

        void initPositionFBO(const int, const int);
        void resizePositionFBO(const int, const int);
};

#endif