#include "VolumeShader.hpp"

#include "Camera.hpp"

#include "glm/gtc/matrix_transform.hpp"

bool VolumeShader::init(int size, glm::vec2 x, glm::vec2 y, glm::vec2 z) {
    if (!Shader::init()) {
        std::cerr << "Error initializing volume shader" << std::endl;
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
    CHECK_GL_CALL(glGenTextures(1, &volumeHandle));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, volumeHandle));

    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    CHECK_GL_CALL(glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, volumeSize, volumeSize, volumeSize, 0, GL_RGBA, GL_FLOAT, nullptr));
    //CHECK_GL_CALL(glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA16F, volumeSize, volumeSize, volumeSize));
    CHECK_GL_CALL(glClearTexImage(volumeHandle, 0, GL_RGBA, GL_FLOAT, nullptr));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, 0));
}


void VolumeShader::voxelize(Mesh *mesh, glm::vec3 position, glm::vec3 scale) {
    CHECK_GL_CALL(glDisable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glDisable(GL_CULL_FACE));
    CHECK_GL_CALL(glDepthMask(GL_FALSE));
    //CHECK_GL_CALL(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));
 
    bind();

    loadMat4(getUniform("P"), &Camera::getP());
    loadMat4(getUniform("V"), &Camera::getV());
    glm::mat4 Vi = Camera::getV();
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    loadMat4(getUniform("Vi"), &Vi);

    glBindImageTexture(1, volumeHandle, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
    loadInt(getUniform("voxelSize"), volumeSize);
    loadVec2(getUniform("xBounds"), xBounds);
    loadVec2(getUniform("yBounds"), yBounds);
    loadVec2(getUniform("zBounds"), zBounds);
 
    /* Bind quad */
    /* VAO */
    CHECK_GL_CALL(glBindVertexArray(mesh->vaoId));

    /* Vertices VBO */
    int pos = getAttribute("vertPos");
    CHECK_GL_CALL(glEnableVertexAttribArray(pos));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, mesh->vertBufId));
    CHECK_GL_CALL(glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr));

    /* Invoke draw call on a quad - this will write to the 3D texture */
    glm::mat4 M  = glm::mat4(1.f);
    //M *= glm::scale(glm::mat4(1.f), scale);
    M *= glm::translate(glm::mat4(1.f), position);
    loadMat4(getUniform("M"), &M);
    CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    /* Wrap up shader */
    glBindVertexArray(0);
    unbind();

    /* Pull 3D texture out of GPU */
    std::vector<float> buffer(volumeSize * volumeSize * volumeSize * 4);
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, volumeHandle));
    CHECK_GL_CALL(glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, buffer.data()));

    // TODO : for x, y, z { find index, load billboard at that index }
    voxelData.clear();
    for (int i = 0; i < buffer.size(); i+=4) {
        float r = buffer[i + 0];
        float g = buffer[i + 1];
        float b = buffer[i + 2];
        float a = buffer[i + 3];
        if (r || g || b || a) {
            voxelData.push_back(glm::vec4(r, g, b, a));
            printf("<%f, %f, %f, %f>\n", r, g, b, a);
        }
    }

    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glEnable(GL_CULL_FACE));
    CHECK_GL_CALL(glDepthMask(GL_TRUE));
    //CHECK_GL_CALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
}
