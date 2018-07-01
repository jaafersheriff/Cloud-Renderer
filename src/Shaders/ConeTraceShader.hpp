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
        float adjustSize = 40.f;
        int minNoiseSteps = 2;
        int maxNoiseSteps = 8;
        float minNoiseColor = 0.2f;
        float noiseColorScale = 0.45f;
        glm::vec3 windVel = glm::vec3(0.01f, 0, 0);

        /* Cone trace parameters */
        int vctSteps = 16;
        float vctConeAngle = 0.9f;
        float vctConeInitialHeight = 0.1f;
        float vctLodOffset = 0.f;
        float vctDownScaling = 1.f;

        bool showQuad = false;
        bool doConeTrace = true;
        bool doNoiseSample = true;

    private:
        void bindVolume(CloudVolume *);
        void unbindVolume();

        void initNoiseMap(int);
        GLuint noiseMapId;
        float totalTime = 0.f;
};

#endif