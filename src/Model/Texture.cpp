#include "Texture.hpp"

#include "Shaders/GLSL.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "ThirdParty/stb_image.h"
#include <iostream>

Texture::Texture(std::string fileName) {
    /* Get texture data */
    uint8_t *data = loadImageData(fileName);

    /* Copy to gpu */
    if (data) {
        copyToGPU(data);
        std::cout << "Loaded texture (" << width << ", " << height << "): " << fileName << std::endl;
    }
    else {
        std::cerr << "Could not find texture file " << fileName << std::endl;
    }

    stbi_image_free(data);
}

uint8_t* Texture::loadImageData(const std::string fileName) {
    stbi_set_flip_vertically_on_load(true);
    return stbi_load(fileName.c_str(), &width, &height, &components, STBI_rgb_alpha);
}

void Texture::copyToGPU(const uint8_t *data) {
    /* Set active texture unit 0 */
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0));

    /* Generate texture buffer object */
    CHECK_GL_CALL(glGenTextures(1, &textureId));

    /* Bind new texture buffer object to active texture */
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, textureId));

    /* Load texture data to GPU */
    CHECK_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));

    /* Generate image pyramid */
    CHECK_GL_CALL(glGenerateMipmap(GL_TEXTURE_2D));
    
    /* Set filtering mode for magnification and minimification */
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));

    /* Set wrap mode */
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));

    /* LOD */
    CHECK_GL_CALL(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, -1.5f)); 

    /* Unbind */
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}