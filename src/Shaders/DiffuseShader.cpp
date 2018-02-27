#include "DiffuseShader.hpp"

#include "Camera.hpp"
#include "Model/Mesh.hpp"

#include "glm/gtc/matrix_transform.hpp"

bool DiffuseShader::init() {
    if (!Shader::init()) {
        std::cerr << "Error initializing billboard shader" << std::endl;
        return false;
    }

    addAttribute("vertPos"); 
    addAttribute("vertNor"); 

    addUniform("P");
    addUniform("M");
    addUniform("V");

    addUniform("lightPos");
}

void DiffuseShader::render(Mesh *mesh, std::vector<glm::vec4> & diffuses, glm::vec3 lightPos) {
    if (!isEnabled) {
        return;
    }

    bind();

    /* Bind projeciton, view, inverise view matrices */
    loadMat4(getUniform("P"), &Camera::getP());
    loadMat4(getUniform("V"), &Camera::getV());

    /* Bind light position */
    loadVec3(getUniform("lightPos"), lightPos);

    /* Bind mesh */
    /* VAO */
    CHECK_GL_CALL(glBindVertexArray(mesh->vaoId));

    /* Vertices VBO */
    int pos = getAttribute("vertPos");
    CHECK_GL_CALL(glEnableVertexAttribArray(pos));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mesh->vertBufId));
    CHECK_GL_CALL(glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr));

    /* Normals VBO */
    pos = getAttribute("vertNor");
    CHECK_GL_CALL(glEnableVertexAttribArray(pos));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mesh->norBufId));
    CHECK_GL_CALL(glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
 
    /* IBO */
    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eleBufId));

    glm::mat4 M;
    for (auto position : diffuses) {
        M  = glm::mat4(1.f);
        M *= glm::translate(glm::mat4(1.f), glm::vec3(position));
        loadMat4(getUniform("M"), &M);

        CHECK_GL_CALL(glDrawElements(GL_TRIANGLES, (int)mesh->eleBuf.size(), GL_UNSIGNED_INT, nullptr));
    }

    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CHECK_GL_CALL(glBindVertexArray(0));

    unbind();
}