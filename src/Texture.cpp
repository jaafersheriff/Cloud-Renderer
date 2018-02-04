#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "thirdparty/stb_image.h"
#include <iostream>

Texture::Texture(std::string fileName) :
    Texture(fileName, GL_REPEAT) {

}

Texture::Texture(std::string fileName, GLenum wrap) {
    if (mode != GL_MIRRORED_REPEAT || mode != GL_CLAMP_TO_EDGE || mode != GL_CLAMP_TO_BORDER || mode != GL_REPEAT) {
        return;
    }

    /* Get texture data */
    uint8_t *data = loadImageData(fileName);

    /* Copy to gpu */
    copyToGPU(data, wrap);

    /* Free texture data */
    delete data;
}

uint8_t* Texture::loadImageData(const std::string fileName) {
    uint8_t *data;

    stbi_set_flip_vertically_on_load(true);
    data = stbi_load(fileName.c_str(), &width, &height, &components, STBI_rgb_alpha);

    if (data) {
        std::cout << "Loaded texture (" << width << ", " << height << "): " << fileName << std::endl;
    }
    else {
        std::cerr << "Could not find texture file " << fileName << std::endl;
    }
    return data;
}

void Texture::copyToGPU(const uint8_t *data, GLenum mode) {
    /* Set active texture unit 0 */
    glActiveTexture(GL_TEXTURE0);

    /* Generate texture buffer object */
    glGenTextures(1, &textureId);

    /* Bind new texture buffer object to active texture */
    glBindTexture(GL_TEXTURE_2D, textureId);

    /* Generate image pyramid */
    glGenerateMipmap(GL_TEXTURE_2D);
    
    /* Set filtering mode for magnification and minimification */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    /* Set wrap mode */
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);

    /* LOD */
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.5f); 

    /* Load texture data to GPU */
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    /* Unbind */
    glBindTexture(GL_TEXTURE_2D, 0);
}