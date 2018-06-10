#include "ConeTraceShader.hpp"

#include "Camera.hpp"
#include "Sun.hpp"
#include "Library.hpp"

ConeTraceShader::ConeTraceShader(const std::string &r, const std::string &v, const std::string &f) :
    Shader(r, v, f) {

    /* Create noise map */
    initNoiseMap(32);
}

void ConeTraceShader::coneTrace(CloudVolume *volume) {
    if (!doConeTrace && !doNoiseSample && !showQuad) {
        return;
    }

    CHECK_GL_CALL(glDisable(GL_DEPTH_TEST));

    bind();
    bindVolume(volume);

    loadVector(getUniform("lightPos"), Sun::position);
    loadBool(getUniform("showQuad"), showQuad);

    /* Bind cone tracing params */
    loadBool(getUniform("doConeTrace"), doConeTrace);
    loadInt(getUniform("vctSteps"), vctSteps);
    loadFloat(getUniform("vctConeAngle"), vctConeAngle);
    loadFloat(getUniform("vctConeInitialHeight"), vctConeInitialHeight);
    loadFloat(getUniform("vctLodOffset"), vctLodOffset);
    loadFloat(getUniform("vctDownScaling"), vctDownScaling);

    /* Bind noise map and params */
    loadBool(getUniform("doNoise"), doNoiseSample);
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0 + noiseMapId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, noiseMapId));
    loadInt(getUniform("noiseMap"), noiseMapId);
    loadFloat(getUniform("stepSize"), stepSize);
    loadFloat(getUniform("noiseOpacity"), noiseOpacity);
    loadInt(getUniform("numOctaves"), numOctaves);
    loadFloat(getUniform("freqStep"), freqStep);
    loadFloat(getUniform("persStep"), persStep);

    /* Per-octave samplign offset */
    std::vector<glm::vec3> octaveOffsets(numOctaves, glm::vec3(0.f));
    for (int i = 0; i < numOctaves; i++) {
        octaveOffsets[i].x = windVel.x * Window::runTime;
        octaveOffsets[i].y = windVel.y * Window::runTime;
        octaveOffsets[i].z = windVel.z * Window::runTime;
    }
    loadVector(getUniform("octaveOffsets"), octaveOffsets.data()[0]);

    /* Bind quad */
    CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));

    /* Bind P V Vi*/
    loadMatrix(getUniform("P"), &Camera::getP());
    loadMatrix(getUniform("V"), &Camera::getV());
    glm::mat4 Vi = Camera::getV();
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    loadMatrix(getUniform("Vi"), &Vi);

    for (const auto &cloudBoard : volume->cloudBoards) {
        /* Cone trace from the camera's perspective */
        loadVector(getUniform("center"), volume->position + cloudBoard.position);
        loadFloat(getUniform("scale"), cloudBoard.scale);

        /* Bind M N */
        glm::mat4 M = glm::translate(glm::mat4(1.f), volume->position + cloudBoard.position);
        M *= glm::scale(glm::mat4(1.f), glm::vec3(cloudBoard.scale));
        loadMatrix(getUniform("M"), &M);
        glm::mat3 N = glm::mat3(transpose(inverse(M * Vi)));
        loadMatrix(getUniform("N"), &N);

        /* Cone trace! */
        CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }

    /* Wrap up shader */
    CHECK_GL_CALL(glBindVertexArray(0));
    unbindVolume();
    unbind();

    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
}

void ConeTraceShader::bindVolume(CloudVolume *volume) {
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0 + volume->volId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, volume->volId));
    loadInt(getUniform("volumeTexture"), volume->volId);

    loadVector(getUniform("xBounds"), volume->position.x + volume->xBounds);
    loadVector(getUniform("yBounds"), volume->position.y + volume->yBounds);
    loadVector(getUniform("zBounds"), volume->position.z + volume->zBounds);
    loadInt(getUniform("voxelDim"), volume->dimension);
}

void ConeTraceShader::unbindVolume() {
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, 0));
}

int getIndex(int x, int y, int z, int dim) {
    if (x < 0)
        x += dim;
    if (y < 0)
        y += dim;
    if (z < 0)
        z += dim;

    x = x % dim;
    y = y % dim;
    z = z % dim;

    return x + y * dim + z * dim * dim;
}

struct CHAR4 { char x, y, z, w; };

float getDensity(int index, CHAR4* pTexels) {
    return (float)pTexels[index].w / 128.0f;
}

void setNormal(glm::vec3 normal, int index, CHAR4* pTexels) {
    pTexels[index].x = (char)(normal.x * 128.0f);
    pTexels[index].y = (char)(normal.y * 128.0f);
    pTexels[index].z = (char)(normal.z * 128.0f);
}

void ConeTraceShader::initNoiseMap(int dimension) {
    CHAR4* pData = new CHAR4[dimension*dimension*dimension];

    /* Populate data */
    for (int i = 0; i < dimension*dimension*dimension; i++) {
        float ran = (float)((rand() % 20000) - 10000) / 10000.f;
        pData[i].w = (char)(ran * 128.0f);
    }

    // Generate normals from the density gradient
    float heightAdjust = 0.5f;
    glm::vec3 normal;
    glm::vec3 densityGradient;
    for (int z = 0; z < dimension; z++) {
        for (int y = 0; y < dimension; y++) {
            for (int x = 0; x < dimension; x++) {
                densityGradient.x = getDensity(getIndex(x + 1, y, z, dimension), pData) - getDensity(getIndex(x - 1, y, z, dimension), pData) / heightAdjust;
                densityGradient.y = getDensity(getIndex(x, y + 1, z, dimension), pData) - getDensity(getIndex(x, y - 1, z, dimension), pData) / heightAdjust;
                densityGradient.z = getDensity(getIndex(x, y, z + 1, dimension), pData) - getDensity(getIndex(x, y, z - 1, dimension), pData) / heightAdjust;
                normal = glm::normalize(densityGradient);
                setNormal(normal, getIndex(x, y, z, dimension), pData);
            }
        }
    }

    CHECK_GL_CALL(glGenTextures(1, &noiseMapId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, noiseMapId));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    CHECK_GL_CALL(glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8_SNORM, dimension, dimension, dimension, 0, GL_RGBA, GL_BYTE, pData));
}
