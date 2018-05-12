/* Abstract parent Shader class 
 * Every feature will have its own derived shader */
#pragma once
#ifndef _SHADER_HPP_
#define _SHADER_HPP_

#include "GLSL.hpp"

#include <glad/glad.h>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <map>
#include <string>
#include <iostream>

class Shader {
    public:
        /* Empty constructor
         * Only used to set GLSL shader names */
        Shader(std::string v = "", std::string f = "") : 
            vShaderName(v), 
            fShaderName(f),
            enabled(true)
        { }

        /* Call parent Shader::init()
         * Add uniforms and attributes to GLSL shaders */
        bool init();

        /* Utility functions */
        void bind();
        void unbind();
        void cleanUp();
        void addAttribute(const std::string &);
        void addUniform(const std::string &);

        /* Parent load functions */
        void loadBool(const int, const bool) const;
        void loadInt(const int, const int) const;
        void loadFloat(const int, const float) const;
        void loadVector(const int, const glm::vec2 &) const;
        void loadVector(const int, const glm::vec3 &) const;
        void loadVector(const int, const glm::vec4 &) const;
        void loadMatrix(const int, const glm::mat4*) const;
        void loadMatrix(const int, const glm::mat3*) const;

        /* Get shader location */
        GLint getAttribute(const std::string &);
        GLint getUniform(const std::string &);

        bool isEnabled() { return enabled; }
        void setEnabled(bool b) { enabled = b; }

    protected:
        /* GLSL shader names */
        const std::string vShaderName;
        const std::string fShaderName;

        bool enabled;
    private:    
        /* GLSL shader attributes */
        GLuint pid = 0;
        GLint vShaderId;
        GLint fShaderId;
        std::map<std::string, GLint> attributes;
        std::map<std::string, GLint> uniforms;
};

#endif
