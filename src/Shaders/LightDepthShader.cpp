#include "LightDepthShader.hpp"

#include "Light.hpp"
#include "Camera.hpp"

#include "glm/gtc/matrix_transform.hpp"

#define DEFAULT_SIZE 1024

LightDepthShader::LightDepthShader(const std::string vert, const std::string frag) :
    Shader(vert, frag) {
    lightMap = new Texture();
}

bool LightDepthShader::init() {
    if (!Shader::init()) {
        return false;
    }

    addAttribute("vertPos");

    addUniform("L");
    addUniform("M");

    lightMap->width = lightMap->height = DEFAULT_SIZE;
    initFBO();

    return true;
}

void LightDepthShader::render(Mesh * mesh, std::vector<Spatial> & spatials) {
    /* Reset light map */
    CHECK_GL_CALL(glViewport(0, 0, lightMap->width, lightMap->height));
    CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fboHandle));
    CHECK_GL_CALL(glClear(GL_DEPTH_BUFFER_BIT));

    bind();
    
    /* Calculate L */
    glm::mat4 lightP = glm::ortho(-10.f, 10.f, -10.f, 10.f, 0.01f, 100.f);
    glm::mat4 lightV = glm::lookAt(Light::spatial.position, glm::vec3(0.f), glm::vec3(0, 1, 0));
    this->L = lightP * lightV;
    loadMat4(getUniform("L"), &L);

    /* Bind mesh */
    /* VAO */
    CHECK_GL_CALL(glBindVertexArray(mesh->vaoId));

    /* Vertices VBO */
    int pos = getAttribute("vertPos");
    if (pos >= 0 && mesh->vertBufId) {
        CHECK_GL_CALL(glEnableVertexAttribArray(pos));
        CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mesh->vertBufId));
        CHECK_GL_CALL(glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
    }

    /* IBO */
    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eleBufId));

    glm::mat4 M;
    for (auto sp : spatials) {
        M  = glm::mat4(1.f);
        M *= glm::translate(glm::mat4(1.f), sp.position);
        M *= glm::scale(glm::mat4(1.f), sp.scale);
        loadMat4(getUniform("M"), &M);

        CHECK_GL_CALL(glDrawElements(GL_TRIANGLES, (int)mesh->eleBuf.size(), GL_UNSIGNED_INT, nullptr));
    }

    unbind();
    CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void LightDepthShader::setTextureSize(int size) {
    lightMap->width = lightMap->height = size;
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, lightMap->textureId));
    CHECK_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, size, size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}

void LightDepthShader::initFBO() {
    /* Generate the FBO for the shadow depth */
    CHECK_GL_CALL(glGenFramebuffers(1, &fboHandle));

    /* Generate the texture */
    glGenTextures(1, &lightMap->textureId);
    glBindTexture(GL_TEXTURE_2D, lightMap->textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, lightMap->width, lightMap->height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    /* Bind with framebuffer's depth buffer */
    CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, fboHandle));
    CHECK_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, lightMap->textureId, 0));
    CHECK_GL_CALL(glDrawBuffer(GL_NONE));
    CHECK_GL_CALL(glReadBuffer(GL_NONE));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}