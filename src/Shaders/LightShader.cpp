#include "LightShader.hpp"

#define DEFAULT_SIZE 1024

LightShader::LightShader(const std::string vert, const std::string frag) :
    Shader(vert, frag) {
    lightMap = new Texture();
}

bool LightShader::init() {
    if (!Shader::init()) {
        return false;
    }

    addAttribute("vertPos");

    lightMap->width = lightMap->height = DEFAULT_SIZE;
    initFBO();

    return true;
}

void LightShader::render() {
    // TODO
}

void LightShader::initFBO() {
    // generate the FBO for the shadow depth
    glGenFramebuffers(1, &fboHandle);

    // generate the texture
    glGenTextures(1, &lightMap->textureId);
    glBindTexture(GL_TEXTURE_2D, lightMap->textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, lightMap->width, lightMap->height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // bind with framebuffer's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, lightMap->textureId, 0);
    glDrawBuffer(GL_NONE);
    //glReadBuffer(GL_NONE);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}