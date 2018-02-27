#include "VolumeShader.hpp"

#include "Camera.hpp"

#include "glm/gtc/matrix_transform.hpp"

bool VolumeShader::init(int size, glm::vec2 x, glm::vec2 y, glm::vec2 z) {
    if (!Shader::init()) {
        return false;
    }

    this->volumeSize = size;
    this->xBounds = x;
    this->yBounds = y;
    this->zBounds = z;

    addAttribute("vertPos");

    addUniform("P");
    addUniform("V");
    addUniform("M");
    addUniform("Vi");

    addUniform("xBounds");
    addUniform("yBounds");
    addUniform("zBounds");
    addUniform("voxelSize");

    addUniform("volume");

    initVolume();

    return true;
}

void VolumeShader::initVolume() {
    glGenTextures(1, &volumeHandle);
    glBindTexture(GL_TEXTURE_3D, volumeHandle);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA16F, volumeSize, volumeSize, volumeSize);
    glClearTexImage(volumeHandle, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_3D, 0);
}


void VolumeShader::voxelize(Mesh *mesh, glm::vec3 position, glm::vec3 scale) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
 
    bind();

    loadMat4(getUniform("P"), &Camera::getP());
    loadMat4(getUniform("V"), &Camera::getV());
    glm::mat4 Vi = Camera::getV();
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    loadMat4(getUniform("Vi"), &Vi);

    glBindImageTexture(1, volumeHandle, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
    loadVec2(getUniform("xBounds"), xBounds);
    loadVec2(getUniform("yBounds"), yBounds);
    loadVec2(getUniform("zBounds"), zBounds);
    loadInt(getUniform("voxelSize"), volumeSize);
 
    /* Bind quad */
    /* VAO */
    glBindVertexArray(mesh->vaoId);

    /* Vertices VBO */
    int pos = getAttribute("vertPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    /* IBO */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->eleBufId);
    
    /* Invoke draw call on a quad - this will write to the 3D texture */
    glm::mat4 M  = glm::mat4(1.f);
    M *= glm::translate(glm::mat4(1.f), glm::vec3(5.f, 0.f, 0.f));
    loadMat4(getUniform("M"), &M);
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDrawElements(GL_TRIANGLES, (int)mesh->eleBuf.size(), GL_UNSIGNED_INT, nullptr);

    /* Wrap up shader */
    glBindVertexArray(0);
    unbind();

    /* Pull 3D texture out of GPU */
    std::vector<float> buffer(volumeSize * volumeSize * volumeSize * 4);
    glBindTexture(GL_TEXTURE_3D, volumeHandle);
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, buffer.data());

    // TODO : for x, y, z { find index, load billboard at that index }
    voxelData.clear();
    for (int i = 0; i < buffer.size(); i+=4) {
        float r = buffer[i + 0];
        float g = buffer[i + 1];
        float b = buffer[i + 2];
        float a = buffer[i + 3];
        if (r || g || b || a) {
            voxelData.push_back(glm::vec4(r, g, b, a));
            printf("<%f, %f, %f, %f>\n", (float)r, float(g), float(b), float(a));
        }
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

