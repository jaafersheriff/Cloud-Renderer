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

    /* Bind quad */
    CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));

    /* Bind light's perspective */
    loadMatrix(getUniform("P"), &Camera::getP());
    loadMatrix(getUniform("V"), &Light::V);
    glm::mat4 Vi = Light::V;
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    loadMatrix(getUniform("Vi"), &Vi);
    loadVector(getUniform("lightPos"), Light::spatial.position);

    /* First voxelize pass 
     * Initial black voxels using billboards
     * Write nearest voxel positions to texture */
    loadInt(getUniform("voxelizeStage"), Voxelize);
    for (auto cloudBoard : volume->cloudBoards) {
        loadVector(getUniform("center"), volume->position + cloudBoard.position);
        loadFloat(getUniform("scale"), cloudBoard.scale.x);
        glm::mat4 M = glm::translate(glm::mat4(1.f), volume->position + cloudBoard.position);
        M *= glm::scale(glm::mat4(1.f), glm::vec3(cloudBoard.scale.x));
        loadMatrix(getUniform("M"), &M);
        glm::mat3 N = glm::mat3(transpose(inverse(M * Vi)));
        loadMatrix(getUniform("N"), &N);
        CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
        glFinish();
    }

    /* Second voxelize pass 
     * Render full screen position texture 
     * Highlight voxels nearest to light */
    loadInt(getUniform("voxelizeStage"), Positions);
    glm::mat4 M = glm::mat4(1.f);
    loadMatrix(getUniform("P"), &M);
    loadMatrix(getUniform("V"), &M);
    loadMatrix(getUniform("Vi"), &M);
    loadMatrix(getUniform("N"), &M);
    M *= glm::scale(glm::mat4(1.f), glm::vec3(2.f));
    loadMatrix(getUniform("M"), &M);
    CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    /* Wrap up shader*/
    CHECK_GL_CALL(glBindVertexArray(0));
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
    CHECK_GL_CALL(glClearTexImage(positionMap->textureId, 0, GL_RGBA, GL_FLOAT, nullptr));
}
