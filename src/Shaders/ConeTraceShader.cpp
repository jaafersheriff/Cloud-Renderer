#include "ConeTraceShader.hpp"

#include "Camera.hpp"
#include "Light.hpp"
#include "Library.hpp"

ConeTraceShader::ConeTraceShader(const std::string &r, const std::string &v, const std::string &f) :
    Shader(r, v, f) {

    /* Create noise map */
    initNoiseMap(32);
}

void ConeTraceShader::coneTrace(Volume *volume, float dt) {
    if (sort) {
        volume->sortBoards(Camera::getPosition());
    }

    CHECK_GL_CALL(glDisable(GL_DEPTH_TEST));

    bind();
    bindVolume(volume);

    loadVector(getUniform("lightPos"), Light::spatial.position);

    /* Bind cone tracing params */
    loadInt(getUniform("vctSteps"), vctSteps);
    loadFloat(getUniform("vctConeAngle"), vctConeAngle);
    loadFloat(getUniform("vctConeInitialHeight"), vctConeInitialHeight);
    loadFloat(getUniform("vctLodOffset"), vctLodOffset);
    loadFloat(getUniform("vctDownScaling"), vctDownScaling);

    /* Bind noise map and params */
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0 + noiseMapId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, noiseMapId));
    loadInt(getUniform("noisemap"), noiseMapId);
    loadFloat(getUniform("g_stepSize"), g_stepSize);
    loadFloat(getUniform("g_noiseOpacity"), g_noiseOpacity);
    loadInt(getUniform("octaves"), numOctaves);
    loadFloat(getUniform("frequency"), freqStep);
    loadFloat(getUniform("persistence"), persStep);

    /* Octaves offsets */
    totalTime += dt * 0.01f;
    std::vector<glm::vec3> octaveOffsets(numOctaves, glm::vec3(0.f));
    for (int i = 0; i < numOctaves; i++) {
        octaveOffsets[i].x = -(float)(totalTime);
        octaveOffsets[i].y = -(float)(totalTime);
        octaveOffsets[i].z = -(float)(totalTime);
    }
    loadVector(getUniform("g_OctaveOffsets"), octaveOffsets.data()[0]);

    /* Bind quad */
    CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));

    /* Bind P V Vi*/
    loadMatrix(getUniform("P"), &Camera::getP());
    loadMatrix(getUniform("V"), &Camera::getV());
    glm::mat4 Vi = Camera::getV();
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    loadMatrix(getUniform("Vi"), &Vi);

    for (auto cloudBoard : volume->cloudBoards) {
        /* Cone trace from the camera's perspective */
        loadVector(getUniform("center"), volume->position + cloudBoard.position);
        loadFloat(getUniform("scale"), cloudBoard.scale.x);

        /* Bind M N */
        glm::mat4 M = glm::translate(glm::mat4(1.f), volume->position + cloudBoard.position);
        M *= glm::scale(glm::mat4(1.f), glm::vec3(cloudBoard.scale.x));
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

void ConeTraceShader::bindVolume(Volume *volume) {
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

void ConeTraceShader::initNoiseMap(int dimension) {
    glm::u8vec4* pData = new glm::u8vec4[dimension*dimension*dimension];

    /* Populate data */
    for (unsigned int i = 0; i < dimension*dimension*dimension; i++) {
        float ran = (float)((rand() % 20000) - 10000) / 10000.f;
        pData[i].w = (glm::uint8)(ran * 128.0f);
    }

    // Generate normals from the density gradient
    float heightAdjust = 0.5f;
    glm::vec3 normal;
    glm::vec3 densityGradient;
    for (unsigned int z = 0; z < dimension; z++) {
        for (unsigned int y = 0; y < dimension; y++) {
            for (unsigned int x = 0; x < dimension; x++) {
                densityGradient.x = getDensity(getIndex(x+1, y, z, dimension), pData) - getDensity(getIndex(x-1, y, z, dimension), pData);
                densityGradient.y = getDensity(getIndex(x, y+1, z, dimension), pData) - getDensity(getIndex(x, y-1, z, dimension), pData);
                densityGradient.z = getDensity(getIndex(x, y, z+1, dimension), pData) - getDensity(getIndex(x, y, z-1, dimension), pData);
                densityGradient /= heightAdjust;
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

float ConeTraceShader::getDensity(int index, glm::u8vec4* pTexels) {
    return (float)pTexels[index].w / 128.0f;
}

void ConeTraceShader::setNormal(glm::vec3 normal, int index, glm::u8vec4* pTexels) {
    pTexels[index].x = (glm::uint8)(normal.x * 128.0f);
    pTexels[index].y = (glm::uint8)(normal.y * 128.0f);
    pTexels[index].z = (glm::uint8)(normal.z * 128.0f);
}
