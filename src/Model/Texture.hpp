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
        Texture() {};

        int width = 0;
        int height = 0;
        int components = 0;

    private:
        uint8_t* loadImageData(const std::string);
        void copyToGPU(const uint8_t *);
};

#endif