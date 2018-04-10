#include "VolumeShader.hpp"

#include "IO/Window.hpp"
#include "Library.hpp"

#include "glm/gtc/matrix_transform.hpp"

bool VolumeShader::init(Volume *vol) {
    if (!Shader::init()) {
        std::cerr << "Error initializing volume shader" << std::endl;
        return false;
    }

    this->volume = vol;

    addAttribute("vertPos");
    
    addUniform("P");
    addUniform("V");
    addUniform("M");
    addUniform("Vi");

    addUniform("xBounds");
    addUniform("yBounds");
    addUniform("zBounds");
    addUniform("dimension");
    addUniform("voxelize");

    addUniform("center");
    addUniform("scale");

    addUniform("normal");

    addUniform("normalStep");
    addUniform("visibilityContrib");

    addUniform("lightMap");
    addUniform("useLightMap");

    addUniform("volume");

    return true;
}

void VolumeShader::voxelize(glm::mat4 P, glm::mat4 V, glm::vec3 camPos, GLuint lightMap) {
    /* Disable quad visualization */
    CHECK_GL_CALL(glDisable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glDisable(GL_CULL_FACE));
    CHECK_GL_CALL(glDepthMask(GL_FALSE));
    CHECK_GL_CALL(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
    
    /* Populate/update volume */
    bind();
    CHECK_GL_CALL(glBindImageTexture(1, volume->volId, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F));
    loadFloat(getUniform("normalStep"), normalStep);
    loadFloat(getUniform("visibilityContrib"), visibilityContrib);
    renderMesh(P, V, camPos, true, lightMap);
    CHECK_GL_CALL(glBindImageTexture(1, 0, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F));
    unbind();

    /* Reset state */
    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glEnable(GL_CULL_FACE));
    CHECK_GL_CALL(glDepthMask(GL_TRUE));
    CHECK_GL_CALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

    /* Update CPU representation of volume */
    volume->updateVoxelData();
}

void VolumeShader::renderMesh(glm::mat4 P, glm::mat4 V, glm::vec3 camPos, bool vox, GLuint lightMap) {
    loadVec3(getUniform("normal"), glm::normalize(camPos - volume->quadPosition));
    loadInt(getUniform("dimension"), volume->dimension);
    loadVec2(getUniform("xBounds"), volume->xBounds);
    loadVec2(getUniform("yBounds"), volume->yBounds);
    loadVec2(getUniform("zBounds"), volume->zBounds);
    loadVec3(getUniform("center"), volume->quadPosition);
    loadFloat(getUniform("scale"), volume->quadScale.x);
    /* Boolean denotes whether to voxelize or not */
    loadBool(getUniform("voxelize"), vox);

    /* Bind quad */
    /* VAO */
    CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));

    /* Vertices VBO */
    // TODO : unnecessary?
    int pos = getAttribute("vertPos");
    CHECK_GL_CALL(glEnableVertexAttribArray(pos));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, Library::quad->vertBufId));
    CHECK_GL_CALL(glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr));

    glm::mat4 M = glm::mat4(1.f);
    /* Render position frame buffer 
     * Update existing voxels based on world positions */
    if (lightMap) {
        /* Bind light map */
        loadInt(getUniform("lightMap"), lightMap);
        CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0 + lightMap));
        CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, lightMap));
        loadBool(getUniform("useLightMap"), true);
        /* Reset matrices so quad renders to full screen */
        loadMat4(getUniform("P"), &M);
        loadMat4(getUniform("V"), &M);
        loadMat4(getUniform("Vi"), &M);
        M *= glm::scale(glm::mat4(1.f), glm::vec3(2.f, 2.f, 1.f));
        loadMat4(getUniform("M"), &M);
        CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
        CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0));
        CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
    }
    else {
        /* Render cloud billboard 
         * Initialize spherical voxels from billboard to light source */
        loadMat4(getUniform("P"), &P);
        loadMat4(getUniform("V"), &V);
        glm::mat4 Vi = V;
        Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
        Vi = glm::transpose(Vi);
        loadMat4(getUniform("Vi"), &Vi);
        loadBool(getUniform("useLightMap"), false);
        M *= glm::translate(glm::mat4(1.f), volume->quadPosition);
        M *= glm::scale(glm::mat4(1.f), glm::vec3(volume->quadScale.x));
        loadMat4(getUniform("M"), &M);
        CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }

    /* Wrap up shader */
    CHECK_GL_CALL(glBindVertexArray(0));
}
