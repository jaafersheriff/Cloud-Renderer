#include "BillboardShader.hpp"

#include "Camera.hpp"
#include "Util.hpp"

bool BillboardShader::init(std::string diffuseName, std::string normalName, Mesh *quad) {
    if (!Shader::init()) {
        std::cerr << "Error initializing billboard shader" << std::endl;
        return false;
    }

    addAttribute("vertPos");

    addUniform("P");
    addUniform("M");
    addUniform("V");
    addUniform("Vi");

    addUniform("diffuseTex");
    addUniform("normalTex");

    addUniform("lightPos");

    this->quad = quad;

    /* Create textures */
    this->diffuseTex = new Texture(diffuseName);
    this->normalTex = new Texture(normalName);
    this->texSize = glm::vec3(diffuseTex->width, diffuseTex->height, 1.f);

    return true;
}

void BillboardShader::render(glm::vec3 lightPos, std::vector<Renderable *> &targets) {
    if (!enabled) {
        return;
    }

    /* Set GL state */
    CHECK_GL_CALL(glDisable(GL_DEPTH_TEST));

    /* Bind shader */
    bind();
    
    /* Bind projeciton, view, inverse view matrices */
    loadMat4(getUniform("P"), &Camera::getP());
    loadMat4(getUniform("V"), &Camera::getV());
    glm::mat4 Vi = Camera::getV();
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    loadMat4(getUniform("Vi"), &Vi);

    /* Bind light position */
    loadVec3(getUniform("lightPos"), lightPos);

    /* Bind mesh */
    /* VAO */
    CHECK_GL_CALL(glBindVertexArray(quad->vaoId));

    /* Vertices VBO */
    int pos = getAttribute("vertPos");
    CHECK_GL_CALL(glEnableVertexAttribArray(pos));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, quad->vertBufId));
    CHECK_GL_CALL(glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr));

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
        M *= glm::scale(glm::mat4(1.f), target->scale * texSize);
        loadMat4(getUniform("M"), &M);

        CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }

    CHECK_GL_CALL(glBindVertexArray(0));
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    unbind();

    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
}

