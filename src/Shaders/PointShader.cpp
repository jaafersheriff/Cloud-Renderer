#include "PointShader.hpp"

#include "Camera.hpp"

bool PointShader::init() {
    if (!Shader::init()) {
        return false;
    }

    addAttribute("vertPos"); 

    addUniform("P");
    addUniform("V");
    addUniform("M");

    /* Init single float geometry */
    // TODO : instance this? Is that even worth it?
    glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    glGenBuffers(1, &vboID);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), nullptr, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return glGetError() == GL_NO_ERROR;
}

void PointShader::render(std::vector<glm::vec4> & points) {
    bind();

    /* Bind projeciton, view, inverise view matrices */
    loadMat4(getUniform("P"), &Camera::getP());
    loadMat4(getUniform("V"), &Camera::getV());

    glBindVertexArray(vaoID);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);

    for (glm::vec3 point : points) {
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3), glm::value_ptr(glm::vec3(point)));
        glDrawArrays(GL_POINT, 0, 1);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    unbind();
}