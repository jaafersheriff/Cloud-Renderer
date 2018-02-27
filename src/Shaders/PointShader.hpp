#pragma once
#ifndef _POINT_SHADER_HPP_
#define _POINT_SHADER_HPP_

#include "Shader.hpp"

#include <vector>

class PointShader : public Shader {
    public:
        PointShader(std::string vertex, std::string fragment) :
            Shader(vertex, fragment)
        {}

        bool init();

        void render(std::vector<glm::vec4> &);

    private:
        GLuint vaoID;
        GLuint vboID;
};

#endif