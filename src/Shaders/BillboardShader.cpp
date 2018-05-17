#include "BillboardShader.hpp"

#include "Library.hpp"
#include "Camera.hpp"
#include "Util.hpp"

BillboardShader::BillboardShader(std::string v, std::string f) :
    Shader(v, f) {
    init();
}

void BillboardShader::render(std::vector<Spatial *> &targets, Texture *diffuseTex, Texture *normalTex) {
    if (!enabled || !diffuseTex || !normalTex) {
        return;
    }

    /* Set GL state */
    CHECK_GL_CALL(glDisable(GL_DEPTH_TEST));

    /* Bind shader */
    bind();
    
    /* Bind projeciton, view, inverse view matrices */
    loadMatrix(getUniform("P"), &Camera::getP());
    loadMatrix(getUniform("V"), &Camera::getV());
    glm::mat4 Vi = Camera::getV();
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    loadMatrix(getUniform("Vi"), &Vi);

    /* Bind light position */
    loadVector(getUniform("lightPos"), Light::spatial.position);

    /* Bind mesh */
    /* VAO */
    CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));

    /* Bind textures */
    loadInt(getUniform("diffuseTex"), diffuseTex->textureId);
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0 + diffuseTex->textureId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, diffuseTex->textureId));
    loadInt(getUniform("normalTex"), normalTex->textureId);
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0 + normalTex->textureId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, normalTex->textureId));

    glm::mat4 M;
    for (auto target : targets) {
        M  = glm::mat4(1.f);
        M *= glm::translate(glm::mat4(1.f), target->position);
        // TODO : fix rotation
        // M *= glm::rotate(glm::mat4(1.f), glm::radians(cloud->rotation), glm::vec3(0, 0, 1));
        glm::vec2 tScale(diffuseTex->width, diffuseTex->height);
        M *= glm::scale(glm::mat4(1.f), target->scale * glm::vec3(glm::normalize(tScale), 1.f));
        loadMatrix(getUniform("M"), &M);

        CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }

    CHECK_GL_CALL(glBindVertexArray(0));
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    unbind();

    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
}

