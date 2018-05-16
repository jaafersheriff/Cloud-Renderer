#include "VoxelizeShader.hpp"

#include "Library.hpp"

#include "Camera.hpp"
#include "Light.hpp"
#include "IO/Window.hpp"

VoxelizeShader::VoxelizeShader(std::string v, std::string f) :
    Shader(v, f) {
    init();

    /* Create position map */
    initPositionMap(Window::width, Window::height);
}

void VoxelizeShader::voxelize(Volume *volume) {
    /* Reset volume and position map */
    volume->clearGPU();
    clearPositionMap();

    /* Disable quad visualization */
    CHECK_GL_CALL(glDisable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glDisable(GL_CULL_FACE));
    CHECK_GL_CALL(glDepthMask(GL_FALSE));
    CHECK_GL_CALL(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));

    bind();
    bindPositionMap();
    bindVolume(volume);

    /* First voxelize pass 
     * Initial black voxelization
     * Write nearest voxel positions to texture */
    for (auto cloudBoard : volume->cloudBoards) {
        renderQuad(volume->position, cloudBoard, Camera::getP(), Light::V, Light::spatial.position, Voxelize);
    }

    glFinish();

    /* Second voxelize pass 
     * Use position texture to highlight voxels nearest to light */
    for (auto cloudBoard : volume->cloudBoards) {
        renderQuad(volume->position, cloudBoard, Camera::getP(), Light::V, Light::spatial.position, Positions);
    }

    CHECK_GL_CALL(glGenerateMipmap(GL_TEXTURE_3D));
    unbindVolume();
    unbindPositionMap();
    unbind();

    /* Reset state */
    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glEnable(GL_CULL_FACE));
    CHECK_GL_CALL(glDepthMask(GL_TRUE));
    CHECK_GL_CALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
}

void VoxelizeShader::renderQuad(glm::vec3 cloudPos, Spatial cloudBoard, glm::mat4 P, glm::mat4 V, glm::vec3 lightPos, Stage stage) {
    loadVector(getUniform("center"), cloudPos + cloudBoard.position);
    loadFloat(getUniform("scale"), cloudBoard.scale.x);
    loadVector(getUniform("lightPos"), Light::spatial.position);

    /* Bind quad */
    CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));

    /* Denotes voxelization stage */
    loadInt(getUniform("voxelizeStage"), stage);

    glm::mat4 M(1.f);
    /* Render a full-screen position map quad for sampling 
     * Implemented in same shader to reuse volume functions */
    if (stage == Positions) {
        loadMatrix(getUniform("P"), &M);
        loadMatrix(getUniform("V"), &M);
        loadMatrix(getUniform("Vi"), &M);
        loadMatrix(getUniform("N"), &M);
        M *= glm::scale(glm::mat4(1.f), glm::vec3(2.f));
        loadMatrix(getUniform("M"), &M);
        CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }
    /* Render cloud billboard from the provided perspective */
    else {
        loadMatrix(getUniform("P"), &P);
        loadMatrix(getUniform("V"), &V);
        glm::mat4 Vi = V;
        Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
        Vi = glm::transpose(Vi);
        loadMatrix(getUniform("Vi"), &Vi);

        M *= glm::translate(glm::mat4(1.f), cloudPos + cloudBoard.position);
        M *= glm::scale(glm::mat4(1.f), glm::vec3(cloudBoard.scale.x));
        loadMatrix(getUniform("M"), &M);

        glm::mat3 N = glm::mat3(transpose(inverse(M * Vi)));
        loadMatrix(getUniform("N"), &N);
        CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    };

    /* Wrap up shader */
    CHECK_GL_CALL(glBindVertexArray(0));
}

void VoxelizeShader::bindVolume(Volume *volume) {
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0 + volume->volId));
    CHECK_GL_CALL(glBindImageTexture(0, volume->volId, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F));
  
    loadVector(getUniform("xBounds"), volume->position.x + volume->xBounds);
    loadVector(getUniform("yBounds"), volume->position.y + volume->yBounds);
    loadVector(getUniform("zBounds"), volume->position.z + volume->zBounds);
    loadInt(getUniform("voxelDim"), volume->dimension);
    loadFloat(getUniform("stepSize"), glm::min(volume->voxelSize.x, glm::min(volume->voxelSize.y, volume->voxelSize.z)));
}

void VoxelizeShader::unbindVolume() {
    CHECK_GL_CALL(glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F));
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0));
}

void VoxelizeShader::bindPositionMap() {
    CHECK_GL_CALL(glBindImageTexture(1, positionMap->textureId, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F));
    loadInt(getUniform("mapWidth"), positionMap->width);
    loadInt(getUniform("mapHeight"), positionMap->height);
}

void VoxelizeShader::unbindPositionMap() {
    CHECK_GL_CALL(glBindImageTexture(1, 0, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F));
}

void VoxelizeShader::initPositionMap(int width, int height) {
    positionMap = new Texture();
    positionMap->width = width;
    positionMap->height = height;
 
    /* Generate the texture */
    CHECK_GL_CALL(glGenTextures(1, &positionMap->textureId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, positionMap->textureId));
    CHECK_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, positionMap->width, positionMap->height, 0, GL_RGBA, GL_FLOAT, NULL));

    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}

void VoxelizeShader::clearPositionMap() {
    glm::vec4 data(FLT_MAX, FLT_MAX, FLT_MAX, 0.f);
    CHECK_GL_CALL(glClearTexImage(positionMap->textureId, 0, GL_RGBA, GL_FLOAT, &data[0]));
}
