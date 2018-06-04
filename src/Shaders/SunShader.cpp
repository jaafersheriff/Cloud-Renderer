#include "SunShader.hpp"

#include "Sun.hpp"
#include "Camera.hpp"
#include "Library.hpp"

void SunShader::render() {
    /* Bind shader */
    bind();
    
    /* Bind projeciton, view, inverse view matrices */
    loadMatrix(getUniform("P"), &Camera::getP());
    loadMatrix(getUniform("V"), &Camera::getV());
    glm::mat4 Vi = Camera::getV();
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    loadMatrix(getUniform("Vi"), &Vi);

    /* Bind mesh */
    /* VAO */
    CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));

    /* M */
    glm::mat4 M = glm::mat4(1.f);
    M *= glm::translate(glm::mat4(1.f), Sun::spatial.position);
    M *= glm::scale(glm::mat4(1.f), Sun::spatial.scale);
    loadMatrix(getUniform("M"), &M);

    /* Draw */
    CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    /* Clean up */
    CHECK_GL_CALL(glBindVertexArray(0));
    unbind();
}

