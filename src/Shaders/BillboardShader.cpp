#include "BillboardShader.hpp"

#include "Camera.hpp"
#include "Cloud.hpp"
#include "Util.hpp"

bool BillboardShader::init(std::string diffuseName, std::string normalName) {
    if (!Shader::init()) {
        return false;
    }

    addAttribute("vertPos");

    addUniform("P");
    addUniform("M");
    addUniform("V");

    addUniform("diffuseTex");
    addUniform("normalTex");

    addUniform("lightPos");

    /* Create quad mesh */
    quad = new Mesh;
    quad->vertBuf = {
        -1.f, -1.f,  0.f,
         1.f, -1.f,  0.f,
        -1.f,  1.f,  0.f,
         1.f,  1.f,  0.f
    };
    quad->init();

    /* Create textures */
    this->diffuseTex = new Texture(diffuseName);
    this->normalTex = new Texture(normalName);

    return true;
}

void BillboardShader::render(glm::vec3 lightPos) {
    /* Set GL state */
    glDisable(GL_DEPTH_TEST);

    /* Bind shader */
    bind();
    
    /* Bind projeciton, view, inverise view matrices */
    loadMat4(getUniform("P"), &Camera::getP());
    loadMat4(getUniform("V"), &Camera::getV());
    glm::mat4 Vi = Camera::getP();
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    loadMat4(getUniform("Vi"), &Vi);

    /* Bind light position */
    loadVec3(getUniform("lightPos"), lightPos);

    /* Bind mesh */
    /* VAO */
    glBindVertexArray(quad->vaoId);

    /* Vertices VBO */
    int pos = getAttribute("vertPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, quad->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    /* IBO */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad->eleBufId);

    /* Bind textures */
    loadInt(getUniform("diffuseTex"), diffuseTex->textureId);
    glActiveTexture(GL_TEXTURE0 + diffuseTex->textureId);
    glBindTexture(GL_TEXTURE_2D, diffuseTex->textureId);
    loadInt(getUniform("normalTex"), normalTex->textureId);
    glActiveTexture(GL_TEXTURE0 + normalTex->textureId);
    glBindTexture(GL_TEXTURE_2D, normalTex->textureId);

    glm::mat4 M;
    for (auto cloud : clouds) {
        M  = glm::mat4(1.f);
        M *= glm::translate(glm::mat4(1.f), cloud->position);
        // TODO : fix M
        // M *= glm::rotate(glm::mat4(1.f), glm::radians(cloud->rotation), glm::vec3(0, 0, 1));
        // M *= glm::scale(glm::mat4(1.f), glm::vec3(cloud->size, 0.f));
        loadMat4(getUniform("M"), &M);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    unbind();
    glEnable(GL_DEPTH_TEST);
}

void BillboardShader::addCloud(glm::vec3 pos) {
    Cloud *cloud = new Cloud;

    cloud->position = pos;
    cloud->size = glm::normalize(glm::vec2(diffuseTex->width, diffuseTex->height));
    // TODO : fix rotation
    cloud->rotation = 0.f;

    clouds.push_back(cloud);
}
