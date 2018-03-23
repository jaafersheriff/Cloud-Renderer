#pragma once
#ifndef _LIGHT_SHADER_HPP_
#define _LIGHT_SHADER_HPP_

#include "Shader.hpp"
#include "Model/Texture.hpp"

class LightShader : public Shader {
    public:
        LightShader(const std::string vert, const std::string frag);

        bool init();

        void render();

    private:
        Texture * lightMap;
        void initFBO();
        GLuint fboHandle;

        glm::mat4 L;
};

#endif