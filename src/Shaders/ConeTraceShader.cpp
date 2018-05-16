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

    /* Bind cone tracing params */
    loadInt(getUniform("vctSteps"), vctSteps);
    loadFloat(getUniform("vctConeAngle"), vctConeAngle);
    loadFloat(getUniform("vctConeInitialHeight"), vctConeInitialHeight);
    loadFloat(getUniform("vctLodOffset"), vctLodOffset);

    loadVector(getUniform("lightPos"), Light::spatial.position);
    loadInt(getUniform("numBoards"), cloud->volumes.size());

    /* Bind quad */
    CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));

    /* Bind P V Vi*/
    loadMatrix(getUniform("P"), &Camera::getP());
    loadMatrix(getUniform("V"), &Camera::getV());
    glm::mat4 Vi = Camera::getV();
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    loadMatrix(getUniform("Vi"), &Vi);

    for (auto volume : cloud->volumes) {
        bindVolume(cloud->spatial.position, volume);

        /* Cone trace from the camera's perspective */
        loadVector(getUniform("center"), cloud->spatial.position + volume->spatial.position);
        loadFloat(getUniform("scale"), volume->spatial.scale.x);

       /* Bind M N */
        glm::mat4 M = glm::translate(glm::mat4(1.f), cloud->spatial.position + volume->spatial.position);
        M *= glm::scale(glm::mat4(1.f), glm::vec3(volume->spatial.scale.x));
        loadMatrix(getUniform("M"), &M);
        glm::mat3 N = glm::mat3(transpose(inverse(M * Vi)));
        loadMatrix(getUniform("N"), &N);

        /* Cone trace! */
        CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

        unbindVolume();
    }

    /* Wrap up shader */
    CHECK_GL_CALL(glBindVertexArray(0));
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
