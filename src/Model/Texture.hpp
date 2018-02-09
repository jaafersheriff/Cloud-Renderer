/* Texture class
 * Contains GL texture ID */
#pragma once
#ifndef _TEXTURE_HPP_
#define _TEXTURE_HPP_

#include <glad/glad.h>

#include <string>
#include <cstdint>

class Texture {
    public:
        /* GL texture ID */
        GLuint textureId = 0;

        Texture(std::string);

        int width;
        int height;
        int components;
        GLenum mode;

    private:
        uint8_t* loadImageData(const std::string);
        void copyToGPU(const uint8_t *);
};

#endif