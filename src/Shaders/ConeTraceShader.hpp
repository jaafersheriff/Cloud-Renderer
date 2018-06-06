#pragma once
#ifndef _CONE_TRACE_SHADER_HPP_
#define _CONE_TRACE_SHADER_HPP_

#include "Shader.hpp"
#include "CloudVolume.hpp"

class ConeTraceShader : public Shader {
    public:
        ConeTraceShader(const std::string &r, const std::string &v, const std::string &f);

        void coneTrace(CloudVolume *);

        /* Noise map parameters */
        float stepSize = 0.01f;
        float noiseOpacity = 4.0;
        int numOctaves = 4;
        float freqStep = 3.f;
        float persStep = 0.5f;
        glm::vec3 windVel = glm::vec3(0.01f, 0, 0);

        /* Cone trace parameters */
        int vctSteps = 16;
        float vctConeAngle = 0.784398163f;
        float vctConeInitialHeight = 0.1f;
        float vctLodOffset = 0.1f;
        float vctDownScaling = 1.f;

        bool doSort = true;
        bool doConeTrace = false;
        bool doNoiseSample = false;

    private:
        void bindVolume(CloudVolume *);
        void unbindVolume();

        void initNoiseMap(int);
        GLuint noiseMapId;
        float totalTime = 0.f;
};

#endif