#include "VolumeShader.hpp"

#include "Camera.hpp"

#include "glm/gtc/matrix_transform.hpp"

bool VolumeShader::init(int size, glm::vec2 x, glm::vec2 y, glm::vec2 z, Spatial *s) {
    if (!Shader::init()) {
        std::cerr << "Error initializing volume shader" << std::endl;
        return false;
    }

    this->volumeSize = size;
    this->xBounds = x;
    this->yBounds = y;
    this->zBounds = z;
    this->volQuad = s;

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
    generateVolume();
    CHECK_GL_CALL(glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, volumeSize, volumeSize, volumeSize, 0, GL_RGBA, GL_FLOAT, nullptr));
}

void VolumeShader::generateVolume() {
    // TODO : does this free previously allocated memory?
    //CHECK_GL_CALL(glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, volumeSize, volumeSize, volumeSize, 0, GL_RGBA, GL_FLOAT, nullptr));
    //CHECK_GL_CALL(glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA32F, volumeSize, volumeSize, volumeSize));
}

void VolumeShader::clearVolume() {
    CHECK_GL_CALL(glClearTexImage(volumeHandle, 0, GL_RGBA, GL_FLOAT, nullptr));
    voxelData.clear();
}

/* bpp - bytes per pixel */
glm::ivec3 VolumeShader::get3DIndices(int index, int bpp) {
	int line = volumeSize * bpp;
	int slice = volumeSize  * line;
	int z = index / slice;
	int y = (index - z * slice) / line;
	int x = index - z * slice - y*line;
	x /= bpp;
	return glm::ivec3(x, y, z);
}

void VolumeShader::voxelize(Mesh *mesh) {
    CHECK_GL_CALL(glDisable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glDisable(GL_CULL_FACE));
    CHECK_GL_CALL(glDepthMask(GL_FALSE));
    CHECK_GL_CALL(glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE));

    /* Generate new volumer per frame*/
    clearVolume();
    generateVolume();

    /* Draw call on mesh to populate volume */
    renderMesh(mesh, true);

    /* Pull volume data out of GPU */
    std::vector<float> buffer(volumeSize * volumeSize * volumeSize * 4);
    CHECK_GL_CALL(glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, buffer.data()));

    for (int i = 0; i < buffer.size(); i+=4) 
    {
    	glm::ivec3 in = get3DIndices(i,4);
        float r = buffer[i + 0];
        float g = buffer[i + 1];
        float b = buffer[i + 2];
        float a = buffer[i + 3];
        if (r || g || b || a) 		
    	{
            voxelData.push_back(glm::vec3(in) - volumeSize/2.f);
        }
    }

    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glEnable(GL_CULL_FACE));
    CHECK_GL_CALL(glDepthMask(GL_TRUE));
    CHECK_GL_CALL(glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE));
}

void VolumeShader::renderMesh(Mesh *mesh, bool voxelize) {
    bind();

    loadMat4(getUniform("P"), &Camera::getP());
    loadMat4(getUniform("V"), &Camera::getV());
    glm::mat4 Vi = Camera::getV();
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    loadMat4(getUniform("Vi"), &Vi);

    glBindImageTexture(1, volumeHandle, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);
    loadInt(getUniform("voxelSize"), volumeSize);
    loadVec2(getUniform("xBounds"), xBounds);
    loadVec2(getUniform("yBounds"), yBounds);
    loadVec2(getUniform("zBounds"), zBounds);
    loadBool(getUniform("voxelize"), voxelize);
 
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
    M *= glm::translate(glm::mat4(1.f), volQuad->position);
    M *= glm::scale(glm::mat4(1.f), glm::vec3(volQuad->scale.x));
    loadMat4(getUniform("M"), &M);
    CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

    /* Wrap up shader */
    glBindVertexArray(0);
    unbind();
}
