#include "VoxelizeShader.hpp"

#include "Library.hpp"

#include "Camera.hpp"
#include "Sun.hpp"
#include "IO/Window.hpp"

VoxelizeShader::VoxelizeShader(const std::string &r, const std::string &v, const std::string &f1, const std::string &f2) {
    /* Initialize first and second pass shaders */
    firstVoxelizer = new Shader(r, v, f1);
    secondVoxelizer = new Shader(r, v, f2);

    /* Create position map */
    initPositionFBO(Window::width, Window::height);
}

void VoxelizeShader::voxelize(CloudVolume *volume) {
    /* Resize position map if window was resized */
    if (Window::width != positionMap->width || Window::height != positionMap->height) {
        resizePositionFBO(Window::width, Window::height);
    }

    /* Reset volume and position map */
    volume->clearGPU();
    clearPositionMap();

    /* Voxelize */
    firstVoxelize(volume);
    secondVoxelize(volume);
}

/* First voxelize pass 
 * Render all billboards and initialize black voxels
 * Write out nearest voxel positions to position FBO */
void VoxelizeShader::firstVoxelize(CloudVolume *volume) {
    /* Bind position FBO */
    CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, positionFBO));
    CHECK_GL_CALL(glClearColor(0.f, 0.f, 0.f, 0.f));
    CHECK_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    firstVoxelizer->bind();

    /* Bind volume and position map */
    bindVolume(firstVoxelizer, volume);

    /* Bind quad */
    CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));

    /* Bind light's perspective */
    firstVoxelizer->loadMatrix(firstVoxelizer->getUniform("P"), &Sun::P);
    firstVoxelizer->loadMatrix(firstVoxelizer->getUniform("V"), &Sun::V);
    glm::mat4 Vi = Sun::V;
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    firstVoxelizer->loadMatrix(firstVoxelizer->getUniform("Vi"), &Vi);
    firstVoxelizer->loadVector(firstVoxelizer->getUniform("lightPos"), Sun::spatial.position);
    firstVoxelizer->loadFloat(firstVoxelizer->getUniform("maxDist"), Sun::maxDist);

    for (const auto &cloudBoard : volume->cloudBoards) {
        /* Bind billboard */
        firstVoxelizer->loadVector(firstVoxelizer->getUniform("center"), volume->position + cloudBoard.position);
        firstVoxelizer->loadFloat(firstVoxelizer->getUniform("scale"), cloudBoard.scale.x);
        glm::mat4 M = glm::translate(glm::mat4(1.f), volume->position + cloudBoard.position);
        M *= glm::scale(glm::mat4(1.f), glm::vec3(cloudBoard.scale.x));
        firstVoxelizer->loadMatrix(firstVoxelizer->getUniform("M"), &M);
        glm::mat3 N = glm::mat3(transpose(inverse(M * Vi)));
        firstVoxelizer->loadMatrix(firstVoxelizer->getUniform("N"), &N);

        /* Draw billboard */
        CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }

    /* Wrap up shader */
    CHECK_GL_CALL(glBindVertexArray(0));
    unbindVolume();
    firstVoxelizer->unbind();

    CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

/* Second voxelize pass 
 * Render position map 
 * Highlight voxels nearest to light */
void VoxelizeShader::secondVoxelize(CloudVolume *volume) {
    /* Disable quad visualization */
    CHECK_GL_CALL(glDisable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glDisable(GL_CULL_FACE));
    CHECK_GL_CALL(glDepthMask(GL_FALSE));
    CHECK_GL_CALL(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));

    secondVoxelizer->bind();

    /* Bind volume */
    bindVolume(secondVoxelizer, volume);

    /* Bind position FBO */
    CHECK_GL_CALL(glBindImageTexture(1, positionMap->textureId, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F));

    /* Bind quad */
    CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));

    /* Bind empty matrices to render full screen quad */
    glm::mat4 M = glm::mat4(1.f);
    secondVoxelizer->loadMatrix(secondVoxelizer->getUniform("P"), &M);
    secondVoxelizer->loadMatrix(secondVoxelizer->getUniform("V"), &M);
    secondVoxelizer->loadMatrix(secondVoxelizer->getUniform("Vi"), &M);
    secondVoxelizer->loadMatrix(secondVoxelizer->getUniform("N"), &M);

    /* Quad goes [-0.5, 0.5], we need it to be [-1, 1] */
    secondVoxelizer->loadMatrix(secondVoxelizer->getUniform("M"), &M);
    
    /* Draw full screen quad */
    CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    /* Generate volume mips now that it is done being updated */
    CHECK_GL_CALL(glGenerateMipmap(GL_TEXTURE_3D));

    /* Wrap up shader*/
    CHECK_GL_CALL(glBindVertexArray(0));
    CHECK_GL_CALL(glBindImageTexture(1, 0, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F));
    unbindVolume();
    secondVoxelizer->unbind();

    /* Reset state */
    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glEnable(GL_CULL_FACE));
    CHECK_GL_CALL(glDepthMask(GL_TRUE));
    CHECK_GL_CALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
}

void VoxelizeShader::bindVolume(Shader *shader, CloudVolume *volume) {
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0 + volume->volId));
    CHECK_GL_CALL(glBindImageTexture(0, volume->volId, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F));
  
    shader->loadVector(shader->getUniform("xBounds"), volume->position.x + volume->xBounds);
    shader->loadVector(shader->getUniform("yBounds"), volume->position.y + volume->yBounds);
    shader->loadVector(shader->getUniform("zBounds"), volume->position.z + volume->zBounds);
    shader->loadInt(shader->getUniform("voxelDim"), volume->dimension);
    shader->loadFloat(shader->getUniform("stepSize"), glm::min(volume->voxelSize.x, glm::min(volume->voxelSize.y, volume->voxelSize.z)));
}

void VoxelizeShader::unbindVolume() {
    CHECK_GL_CALL(glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F));
    CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0));
}

void VoxelizeShader::initPositionFBO(const int width, const int height) {
    /* Generate FBO */
    CHECK_GL_CALL(glGenFramebuffers(1, &positionFBO));
    CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, positionFBO));

    /* Generate color attachment texture */
    positionMap = new Texture();
    positionMap->width = width;
    positionMap->height = height;
    CHECK_GL_CALL(glGenTextures(1, &positionMap->textureId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, positionMap->textureId));
    CHECK_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, positionMap->width, positionMap->height, 0, GL_RGBA, GL_FLOAT, NULL));
    CHECK_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, positionMap->textureId, 0));

    /* Texture params */
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    /* Generate depth attachment texture */
    depthMap = new Texture();
    depthMap->width = width;
    depthMap->height = height;
    CHECK_GL_CALL(glGenRenderbuffers(1, &depthMap->textureId));
    CHECK_GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, depthMap->textureId));
    CHECK_GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, depthMap->width, depthMap->height));
    CHECK_GL_CALL(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthMap->textureId));

    /* Clean up */
    CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void VoxelizeShader::resizePositionFBO(const int width, const int height) {
    /* Resize color attachment */
    positionMap->width = width;
    positionMap->height = height;
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, positionMap->textureId));
    CHECK_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, positionMap->width, positionMap->height, 0, GL_RGBA, GL_FLOAT, NULL));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));

    /* Resize depth attachment */
    depthMap->width = width;
    depthMap->height = height;
    CHECK_GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, depthMap->textureId));
    CHECK_GL_CALL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, depthMap->width, depthMap->height));
    CHECK_GL_CALL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
}

void VoxelizeShader::clearPositionMap() {
    CHECK_GL_CALL(glClearTexImage(positionMap->textureId, 0, GL_RGBA, GL_FLOAT, nullptr));
}