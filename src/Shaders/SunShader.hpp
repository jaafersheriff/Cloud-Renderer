#pragma once
#ifndef _SUN_SHADER_HPP_
#define _SUN_SHADER_HPP_

#include "Shader.hpp"

class SunShader : public Shader {
    public:
        SunShader(const std::string &r, const std::string &v, const std::string &f) :
            Shader(r, v, f) 
        {}

        void render();
};

#endif