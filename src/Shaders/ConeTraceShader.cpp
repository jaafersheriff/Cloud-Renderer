#include "ConeTraceShader.hpp"

#include "Camera.hpp"
#include "Light.hpp"
#include "Library.hpp"

bool ConeTraceShader::init() {
    if (!Shader::init()) {
        std::cerr << "Error initalizing cone trace shader" << std::endl;
        return false;
    }

    addAttribute("vertPos");
    
    addUniform("P");
    addUniform("V");
    addUniform("M");
    addUniform("Vi");

    addUniform("center");
    addUniform("scale");

    addUniform("voxelDim");
    addUniform("xBounds");
    addUniform("yBounds");
    addUniform("zBounds");
    addUniform("lightPos");

    addUniform("volumeTexture");
    addUniform("vctSteps");
    addUniform("vctBias");
    addUniform("vctConeAngle");
    addUniform("vctConeInitialHeight");
    addUniform("vctLodOffset");
}

void ConeTraceShader::coneTrace(Volume *volume) {
    bind();
    bindVolume(volume);

    /* Bind cone tracing params */
    loadInt(getUniform("vctSteps"), vctSteps);
    loadFloat(getUniform("vctBias"), vctBias);
    loadFloat(getUniform("vctConeAngle"), vctConeAngle);
    loadFloat(getUniform("vctConeInitialHeight"), vctConeInitialHeight);
    loadFloat(getUniform("vctLodOffset"), vctLodOffset);

    /* Cone trace from the camera's perspective */
    loadVec3(getUniform("center"), volume->quadPosition);
    loadFloat(getUniform("scale"), volume->quadScale.x);
    loadVec3(getUniform("lightPos"), Light::spatial.position);

    /* Bind quad */
    /* VAO */
    CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));

    /* Vertices VBO */
    // TODO : unnecessary?
    int pos = getAttribute("vertPos");
    CHECK_GL_CALL(glEnableVertexAttribArray(pos));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, Library::quad->vertBufId));
    CHECK_GL_CALL(glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr));

    loadMat4(getUniform("P"), &Camera::getP());
    loadMat4(getUniform("V"), &Camera::getV());
    glm::mat4 Vi = Camera::getV();
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    loadMat4(getUniform("Vi"), &Vi);

    glm::mat4 M = glm::translate(glm::mat4(1.f), volume->quadPosition);
    M *= glm::scale(glm::mat4(1.f), glm::vec3(volume->quadScale.x));
    loadMat4(getUniform("M"), &M);
    CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    /* Wrap up shader */
    CHECK_GL_CALL(glBindVertexArray(0));

    unbindVolume();
    unbind();
}

void ConeTraceShader::bindVolume(Volume *volume) {
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0 + volume->volId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, volume->volId));
    loadInt(getUniform("volumeTexture"), volume->volId);
    
    loadInt(getUniform("voxelDim"), volume->dimension);
    loadVec2(getUniform("xBounds"), volume->xBounds);
    loadVec2(getUniform("yBounds"), volume->yBounds);
    loadVec2(getUniform("zBounds"), volume->zBounds);
}

void ConeTraceShader::unbindVolume() {
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, 0));
}
