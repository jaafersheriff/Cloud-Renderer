#include "PointShader.hpp"

#include "Camera.hpp"

bool PointShader::init() {
    if (!Shader::init()) {
        std::cerr << "Error initializing billboard shader" << std::endl;
        return false;
    }

    addAttribute("vertPos"); 

    addUniform("P");
    addUniform("V");

    /* Init single float geometry */
    // TODO : instance this? Is that even worth it?
    CHECK_GL_CALL(glGenVertexArrays(1, &vaoID));
    CHECK_GL_CALL(glBindVertexArray(vaoID));

    CHECK_GL_CALL(glGenBuffers(1, &vboID));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vboID));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3), nullptr, GL_STATIC_DRAW));

    CHECK_GL_CALL(glBindVertexArray(0));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void PointShader::render(std::vector<glm::vec4> & points) {
    if (!isEnabled) {
        return;
    }

    bind();

    /* Bind projeciton, view, inverise view matrices */
    loadMat4(getUniform("P"), &Camera::getP());
    loadMat4(getUniform("V"), &Camera::getV());

    CHECK_GL_CALL(glBindVertexArray(vaoID));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vboID));

    for (glm::vec3 point : points) {
        CHECK_GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::vec3), glm::value_ptr(glm::vec3(point))));
        CHECK_GL_CALL(glDrawArrays(GL_POINT, 0, 1));
    }

    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CHECK_GL_CALL(glBindVertexArray(0));

    unbind();
}