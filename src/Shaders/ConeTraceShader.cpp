#include "ConeTraceShader.hpp"

#include "Camera.hpp"
#include "Light.hpp"
#include "Library.hpp"

ConeTraceShader::ConeTraceShader(std::string v, std::string f) :
    Shader(v, f) {
    init();
}

void ConeTraceShader::coneTrace(Cloud *cloud) {
    bind();
    for (auto volume : cloud->volumes) {
        bindVolume(cloud->spatial.position, volume);

        /* Bind cone tracing params */
        loadInt(getUniform("vctSteps"), vctSteps);
        loadFloat(getUniform("vctConeAngle"), vctConeAngle);
        loadFloat(getUniform("vctConeInitialHeight"), vctConeInitialHeight);
        loadFloat(getUniform("vctLodOffset"), vctLodOffset);

        /* Cone trace from the camera's perspective */
        loadVector(getUniform("center"), cloud->spatial.position + volume->spatial.position);
        loadFloat(getUniform("scale"), volume->spatial.scale.x);
        loadVector(getUniform("lightPos"), Light::spatial.position);

        /* Bind quad */
        // TODO : no need to rebind buffers and attrib pointers
        CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));
        int pos = getAttribute("vertPos");
        CHECK_GL_CALL(glEnableVertexAttribArray(pos));
        CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, Library::quad->vertBufId));
        CHECK_GL_CALL(glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
        pos = getAttribute("vertNor");
        CHECK_GL_CALL(glEnableVertexAttribArray(pos));
        CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, Library::quad->norBufId));
        CHECK_GL_CALL(glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr));

        loadMatrix(getUniform("P"), &Camera::getP());
        loadMatrix(getUniform("V"), &Camera::getV());
        glm::mat4 Vi = Camera::getV();
        Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
        Vi = glm::transpose(Vi);
        loadMatrix(getUniform("Vi"), &Vi);

        glm::mat4 M = glm::translate(glm::mat4(1.f), cloud->spatial.position + volume->spatial.position);
        M *= glm::scale(glm::mat4(1.f), glm::vec3(volume->spatial.scale.x));
        loadMatrix(getUniform("M"), &M);

        glm::mat3 N = glm::mat3(transpose(inverse(M * Vi)));
        loadMatrix(getUniform("N"), &N);
        CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

        /* Wrap up shader */
        CHECK_GL_CALL(glBindVertexArray(0));

        unbindVolume();
    }
    unbind();
}

void ConeTraceShader::bindVolume(glm::vec3 cloudPos, Volume *volume) {
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0 + volume->volId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, volume->volId));
    loadInt(getUniform("volumeTexture"), volume->volId);
    
    loadVector(getUniform("xBounds"), cloudPos.x + volume->xBounds);
    loadVector(getUniform("yBounds"), cloudPos.y + volume->yBounds);
    loadVector(getUniform("zBounds"), cloudPos.z + volume->zBounds);
    loadInt(getUniform("voxelDim"), volume->dimension);
}

void ConeTraceShader::unbindVolume() {
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, 0));
}
