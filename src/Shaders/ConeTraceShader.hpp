#pragma once
#ifndef _CONE_TRACE_SHADER_HPP_
#define _CONE_TRACE_SHADER_HPP_

#include "Shader.hpp"
#include "Cloud.hpp"

class ConeTraceShader : public Shader {
    public:
        ConeTraceShader(std::string, std::string);

        void coneTrace(Cloud *);

        /* Cone trace parameters */
        int vctSteps = 16;
        float vctConeAngle = 0.784398163f;
        float vctConeInitialHeight = 0.1f;
        float vctLodOffset = 0.1f;

    private:
        void bindVolume(glm::vec3, Volume *);
        void unbindVolume();
};

#endif