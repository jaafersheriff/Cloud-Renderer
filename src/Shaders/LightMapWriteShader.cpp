#include "LightMapWriteShader.hpp"

#include "IO/Window.hpp"
#include "Library.hpp"

#include "Light.hpp"
#include "Camera.hpp"

#include "glm/gtc/matrix_transform.hpp"

#define DEFAULT_SIZE 1024

LightMapWriteShader::LightMapWriteShader(const std::string vert, const std::string frag) :
    Shader(vert, frag) {
    lightMap = new Texture();
}

bool LightMapWriteShader::init() {
    if (!Shader::init()) {
        return false;
    }

    addAttribute("vertPos");

    addUniform("P");
    addUniform("V");
    addUniform("M");

    lightMap->width = lightMap->height = DEFAULT_SIZE;
    initFBO();

    return true;
}

void LightMapWriteShader::render(std::vector<VolumeShader::Voxel> & voxels) {
    /* Reset light map */
    CHECK_GL_CALL(glViewport(0, 0, lightMap->width, lightMap->height));
    CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fboHandle));
    CHECK_GL_CALL(glClearColor(0.f, 0.f, 0.f, 0.f));
    CHECK_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    bind();
    
    /* Load light perspective */
    loadMat4(getUniform("P"), &Light::P);
    loadMat4(getUniform("V"), &Light::V);

    /* Bind Library::cube */
    /* VAO */
    CHECK_GL_CALL(glBindVertexArray(Library::cube->vaoId));

    /* Vertices VBO */
    int pos = getAttribute("vertPos");
    if (pos >= 0 && Library::cube->vertBufId) {
        CHECK_GL_CALL(glEnableVertexAttribArray(pos));
        CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, Library::cube->vertBufId));
        CHECK_GL_CALL(glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
    }

    /* IBO */
    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Library::cube->eleBufId));

    glm::mat4 M;
    for (auto v : voxels) {
        if (v.data.r || v.data.g || v.data.b || v.data.a) {
            M  = glm::mat4(1.f);
            M *= glm::translate(glm::mat4(1.f), v.spatial.position);
            M *= glm::scale(glm::mat4(1.f), v.spatial.scale);
            loadMat4(getUniform("M"), &M);

            CHECK_GL_CALL(glDrawElements(GL_TRIANGLES, (int)Library::cube->eleBuf.size(), GL_UNSIGNED_INT, nullptr));
        }
    }

    CHECK_GL_CALL(glBindVertexArray(0));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    unbind();
    CHECK_GL_CALL(glViewport(0, 0, Window::width, Window::height));
    CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void LightMapWriteShader::initFBO() {
    /* Generate the FBO for the shadow depth */
    CHECK_GL_CALL(glGenFramebuffers(1, &fboHandle));

    /* Generate the texture */
    CHECK_GL_CALL(glGenTextures(1, &lightMap->textureId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, lightMap->textureId));
    CHECK_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, lightMap->width, lightMap->height, 0, GL_RGB, GL_FLOAT, NULL));

    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    /* Bind with framebuffer's depth buffer */
    CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fboHandle));
    CHECK_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightMap->textureId, 0));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}