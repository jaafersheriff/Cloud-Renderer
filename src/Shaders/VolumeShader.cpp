#include "VolumeShader.hpp"

#include "IO/Window.hpp"
#include "Library.hpp"
#include "Camera.hpp"

#include "glm/gtc/matrix_transform.hpp"

bool VolumeShader::init(int size, glm::vec2 x, glm::vec2 y, glm::vec2 z, Spatial *s) {
    for (int i = 0; i < size*size*size; i++) {
        this->voxelData.push_back({ Spatial(glm::vec3(0.f), glm::vec3(0.f), glm::vec3(0.f)), glm::vec4(0.f) });
    }
    this->volumeSize = size;
    this->xBounds = x;
    this->yBounds = y;
    this->zBounds = z;
    this->volQuad = s;

    if (!Shader::init()) {
        std::cerr << "Error initializing volume shader" << std::endl;
        return false;
    }

    addAttribute("vertPos");
    
    addUniform("P");
    addUniform("V");
    addUniform("M");
    addUniform("Vi");

    addUniform("xBounds");
    addUniform("yBounds");
    addUniform("zBounds");
    addUniform("voxelSize");
    addUniform("voxelize");

    addUniform("center");
    addUniform("scale");

    addUniform("normal");

    addUniform("normalStep");
    addUniform("visibilityContrib");

    addUniform("lightMap");
    addUniform("useLightMap");

    addUniform("volume");

    initVolume();

    return true;
}

void VolumeShader::initVolume() {
    CHECK_GL_CALL(glGenTextures(1, &volumeHandle));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, volumeHandle));

    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA16F, volumeSize, volumeSize, volumeSize));
    clearVolume();
    //CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, 0));
}

void VolumeShader::clearVolume() {
    CHECK_GL_CALL(glClearTexImage(volumeHandle, 0, GL_RGBA, GL_FLOAT, nullptr));
    for (unsigned int i = 0; i < voxelData.size(); i++) {
        voxelData[i].spatial.position = glm::vec3(0.f);
        voxelData[i].spatial.scale = glm::vec3(0.f);
        voxelData[i].data = glm::vec4(0.f);
    }
}

void VolumeShader::voxelize(glm::mat4 P, glm::mat4 V, glm::vec3 camPos, GLuint lightMap) {
    CHECK_GL_CALL(glDisable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glDisable(GL_CULL_FACE));
    CHECK_GL_CALL(glDepthMask(GL_FALSE));
    CHECK_GL_CALL(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
    
    /* Draw call on mesh to populate volume */
    bind();
    CHECK_GL_CALL(glBindImageTexture(1, volumeHandle, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F));
    loadFloat(getUniform("normalStep"), normalStep);
    loadFloat(getUniform("visibilityContrib"), visibilityContrib);
    renderMesh(P, V, camPos, true, lightMap);
    CHECK_GL_CALL(glBindImageTexture(1, 0, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F));
    unbind();
    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glEnable(GL_CULL_FACE));
    CHECK_GL_CALL(glDepthMask(GL_TRUE));
    CHECK_GL_CALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));

    updateVoxelData();
}

void VolumeShader::renderMesh(glm::mat4 P, glm::mat4 V, glm::vec3 camPos, bool vox, GLuint lightMap) {
    loadInt(getUniform("voxelSize"), volumeSize);
    loadVec2(getUniform("xBounds"), xBounds);
    loadVec2(getUniform("yBounds"), yBounds);
    loadVec2(getUniform("zBounds"), zBounds);
    loadBool(getUniform("voxelize"), vox);
    loadVec3(getUniform("center"), volQuad->position);
    loadFloat(getUniform("scale"), volQuad->scale.x);
    loadVec3(getUniform("normal"), glm::normalize(camPos - volQuad->position));

    /* Bind quad */
    /* VAO */
    CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));

    /* Vertices VBO */
    int pos = getAttribute("vertPos");
    CHECK_GL_CALL(glEnableVertexAttribArray(pos));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, Library::quad->vertBufId));
    CHECK_GL_CALL(glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr));

    glm::mat4 M = glm::mat4(1.f);
    if (lightMap) {
        /* Render full screen quad 
         * Reuse frag voxelize functionality to update voxels nearest to camera */
        loadInt(getUniform("lightMap"), lightMap);
        CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0 + lightMap));
        CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, lightMap));
        loadBool(getUniform("useLightMap"), true);
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
         * 'Tag' spherical voxels from billboard to light source */
        loadMat4(getUniform("P"), &P);
        loadMat4(getUniform("V"), &V);
        glm::mat4 Vi = V;
        Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
        Vi = glm::transpose(Vi);
        loadMat4(getUniform("Vi"), &Vi);
        loadBool(getUniform("useLightMap"), false);
        M *= glm::translate(glm::mat4(1.f), volQuad->position);
        M *= glm::scale(glm::mat4(1.f), glm::vec3(volQuad->scale.x));
        loadMat4(getUniform("M"), &M);
        CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }

    /* Wrap up shader */
    CHECK_GL_CALL(glBindVertexArray(0));
}

void VolumeShader::updateVoxelData() { 
    /* Pull volume data out of GPU */
    std::vector<float> buffer(voxelData.size() * 4);
    CHECK_GL_CALL(glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, buffer.data()));

    glm::vec3 voxelScale = glm::vec3(
                (xBounds.y - xBounds.x) / volumeSize,
                (yBounds.y - yBounds.x) / volumeSize,
                (zBounds.y - zBounds.x) / volumeSize);
    for (unsigned int i = 0; i < voxelData.size(); i++) {
        float r = buffer[4*i + 0];
        float g = buffer[4*i + 1];
        float b = buffer[4*i + 2];
        float a = buffer[4*i + 3];
        if (r || g || b || a) {
            glm::ivec3 in = get3DIndices(4*i);       // voxel index 
            glm::vec3 wPos = reverseVoxelIndex(in);  // world space
            voxelData[i].spatial.position = wPos;
            voxelData[i].spatial.scale = voxelScale;
            voxelData[i].data = glm::vec4(r, g, b, a);
        }
    }
}

// Assume 4 bytes per voxel
glm::ivec3 VolumeShader::get3DIndices(int index) {
	int line = volumeSize * 4;
	int slice = volumeSize  * line;
	int z = index / slice;
	int y = (index - z * slice) / line;
	int x = (index - z * slice - y * line) / 4;
	return glm::ivec3(x, y, z);
}

glm::vec3 VolumeShader::reverseVoxelIndex(glm::ivec3 voxelIndex) {
    float xRange = xBounds.y - xBounds.x;
    float yRange = yBounds.y - yBounds.x;
    float zRange = zBounds.y - zBounds.x;

    float x = float(voxelIndex.x) * xRange / volumeSize + xBounds.x;
    float y = float(voxelIndex.y) * yRange / volumeSize + yBounds.x;
    float z = float(voxelIndex.z) * zRange / volumeSize + zBounds.x;

    return glm::vec3(x, y, z);
}