#include "Volume.hpp"

#include "Shaders/GLSL.hpp"

Volume::Volume(int dim, glm::vec2 xBounds, glm::vec2 yBounds, glm::vec2 zBounds, glm::vec3 position, glm::vec2 size) {
    for (int i = 0; i < dim*dim*dim; i++) {
        this->voxelData.push_back({
            Spatial(),
            glm::vec4()
            });
    }
    this->dimension = dim;
    this->quadPosition = position;
    this->quadScale = size;
    this->xBounds = xBounds;
    this->yBounds = yBounds;
    this->zBounds = zBounds;

    /* Init volume */
    CHECK_GL_CALL(glGenTextures(1, &volId));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, volId));

    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    CHECK_GL_CALL(glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA16F, dimension, dimension, dimension));
    clear();
    //CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, 0));
}

void Volume::clear() {
    /* Reset GPU volume */
    CHECK_GL_CALL(glClearTexImage(volId, 0, GL_RGBA, GL_FLOAT, nullptr));

    /* Reset CPU representation of volume */
    for (unsigned int i = 0; i < voxelData.size(); i++) {
        voxelData[i].spatial.position = glm::vec3(0.f);
        voxelData[i].spatial.scale = glm::vec3(0.f);
        voxelData[i].data = glm::vec4(0.f);
    }
}

void Volume::updateVoxelData() {
    /* Pull volume data out of GPU */
    std::vector<float> buffer(voxelData.size() * 4);
    CHECK_GL_CALL(glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, buffer.data()));

    /* Size of voxels in world-space */
    glm::vec3 voxelScale = glm::vec3(
                (xBounds.y - xBounds.x) / dimension,
                (yBounds.y - yBounds.x) / dimension,
                (zBounds.y - zBounds.x) / dimension);
    voxelCount = 0;
    for (unsigned int i = 0; i < voxelData.size(); i++) {
        float r = buffer[4*i + 0];
        float g = buffer[4*i + 1];
        float b = buffer[4*i + 2];
        float a = buffer[4*i + 3];
        /* Update voxel data if data exists */
        if (r || g || b || a) {
            voxelCount++;
            glm::ivec3 in = get3DIndices(4*i);       // voxel index 
            glm::vec3 wPos = reverseVoxelIndex(in);  // world space
            voxelData[i].spatial.position = wPos;
            voxelData[i].spatial.scale = voxelScale;
            voxelData[i].data = glm::vec4(r, g, b, a);
        }
    }
}

// Assume 4 bytes per voxel
glm::ivec3 Volume::get3DIndices(int index) {
	int line = dimension * 4;
	int slice = dimension  * line;
	int z = index / slice;
	int y = (index - z * slice) / line;
	int x = (index - z * slice - y * line) / 4;
	return glm::ivec3(x, y, z);
}

glm::vec3 Volume::reverseVoxelIndex(glm::ivec3 voxelIndex) {
    float xRange = xBounds.y - xBounds.x;
    float yRange = yBounds.y - yBounds.x;
    float zRange = zBounds.y - zBounds.x;

    float x = float(voxelIndex.x) * xRange / dimension + xBounds.x;
    float y = float(voxelIndex.y) * yRange / dimension + yBounds.x;
    float z = float(voxelIndex.z) * zRange / dimension + zBounds.x;

    return glm::vec3(x, y, z);
}
