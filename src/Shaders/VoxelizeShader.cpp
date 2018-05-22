#include "VoxelizeShader.hpp"

#include "Library.hpp"

#include "Camera.hpp"
#include "Light.hpp"
#include "IO/Window.hpp"

VoxelizeShader::VoxelizeShader(std::string v, std::string f1, std::string f2) {
    /* Initialize first and second pass shaders */
    firstVoxelizer = new Shader(v, f1);
    firstVoxelizer->init();

    secondVoxelizer = new Shader(v, f2);
    secondVoxelizer->init();

    /* Create position FBO */
    glGenFramebuffers(1, &positionFBO);
    positionTexture = new Texture();
    positionTexture->width = Window::width;
    positionTexture->height = Window::height;
    CHECK_GL_CALL(glGenTextures(1, &positionTexture->textureId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, positionTexture->textureId));
    CHECK_GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, positionTexture->width, positionTexture->height, 0, GL_RGBA, GL_FLOAT, NULL));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, positionFBO));
    CHECK_GL_CALL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, positionTexture->textureId, 0));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
}

void VoxelizeShader::voxelize(Volume *volume) {
    /* Reset volume and position map */
    volume->clearGPU();

    /* Voxelize */
    firstVoxelize(volume);
    secondVoxelize(volume);
}

/* First voxelize pass 
 * Render all billboards and initialize black voxels
 * Write out nearest voxel positions to FBO */
void VoxelizeShader::firstVoxelize(Volume *volume) {
    /* Bind position map FBO */
    CHECK_GL_CALL(glViewport(0, 0, Window::width, Window::height));
    CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, positionFBO));
    CHECK_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    firstVoxelizer->bind();

    /* Bind volume */
    bindVolume(firstVoxelizer, volume);

    /* Bind quad */
    CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));

    /* Bind light's perspective */
    firstVoxelizer->loadMatrix(firstVoxelizer->getUniform("P"), &Camera::getP());
    firstVoxelizer->loadMatrix(firstVoxelizer->getUniform("V"), &Light::V);
    glm::mat4 Vi = Light::V;
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    firstVoxelizer->loadMatrix(firstVoxelizer->getUniform("Vi"), &Vi);
    firstVoxelizer->loadVector(firstVoxelizer->getUniform("lightPos"), Light::spatial.position);

    for (auto cloudBoard : volume->cloudBoards) {
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

        // TODO glflush/glfinish
    }

    /* Wrap up shader */
    CHECK_GL_CALL(glBindVertexArray(0));
    firstVoxelizer->unbind();
    
    /* Unbind position map FBO */
    CHECK_GL_CALL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

/* Second voxelize pass 
 * Render position FBO 
 * Highlight voxels nearest to light */
void VoxelizeShader::secondVoxelize(Volume *volume) {
    /* Disable quad visualization */
    CHECK_GL_CALL(glDisable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glDisable(GL_CULL_FACE));
    CHECK_GL_CALL(glDepthMask(GL_FALSE));
    CHECK_GL_CALL(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));

    secondVoxelizer->bind();

    /* Bind volume */
    bindVolume(secondVoxelizer, volume);

    /* Bind quad */
    CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));

    /* Bind empty matrices to render full screen quad */
    glm::mat4 M = glm::mat4(1.f);
    secondVoxelizer->loadMatrix(secondVoxelizer->getUniform("P"), &M);
    secondVoxelizer->loadMatrix(secondVoxelizer->getUniform("V"), &M);
    secondVoxelizer->loadMatrix(secondVoxelizer->getUniform("Vi"), &M);
    secondVoxelizer->loadMatrix(secondVoxelizer->getUniform("N"), &M);

    /* Quad goes [-0.5, 0.5], we need it to be [-1, 1] */
    M *= glm::scale(glm::mat4(1.f), glm::vec3(2.f));
    secondVoxelizer->loadMatrix(secondVoxelizer->getUniform("M"), &M);
    
    /* Draw full screen quad */
    CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    /* Generate volume mips now that it is done being updated */
    CHECK_GL_CALL(glGenerateMipmap(GL_TEXTURE_3D));

    /* Wrap up shader*/
    CHECK_GL_CALL(glBindVertexArray(0));
    unbindVolume();
    secondVoxelizer->unbind();

    /* Reset state */
    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glEnable(GL_CULL_FACE));
    CHECK_GL_CALL(glDepthMask(GL_TRUE));
    CHECK_GL_CALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
}

void VoxelizeShader::bindVolume(Shader *shader, Volume *volume) {
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
