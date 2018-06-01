#pragma once
#ifndef _CONE_TRACE_SHADER_HPP_
#define _CONE_TRACE_SHADER_HPP_

#include "Shader.hpp"
#include "Volume.hpp"

class ConeTraceShader : public Shader {
    public:
        ConeTraceShader(const std::string &r, const std::string &v, const std::string &f) :
            Shader(r, v, f)
        {
            genNoise(32);
        }

        void coneTrace(Volume *, float);

        /* Noise map parameters */
        float g_stepSize = 0.01;
        float g_noiseOpacity = 4.0;
        int octaves = 4;
        float frequency = 3;
        float persistence = 0.5;

        /* Cone trace parameters */
        int vctSteps = 16;
        float vctConeAngle = 0.784398163f;
        float vctConeInitialHeight = 0.1f;
        float vctLodOffset = 0.1f;
        float vctDownScaling = 1.f;

        bool sort = true;

    private:
        void bindVolume(Volume *);
        void unbindVolume();


        void genNoise(int);
        GLuint TexNoise;
};

#endif