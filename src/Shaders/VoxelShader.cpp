#include "VoxelShader.hpp"

#include "Library.hpp"
#include "Camera.hpp"
#include "Model/Mesh.hpp"

#include "glm/gtc/matrix_transform.hpp"

VoxelShader::VoxelShader(int dimension, const std::string &r, const std::string &v, const std::string &f) :
    Shader(r, v, f) {
    int numVoxels = dimension * dimension * dimension;

    /* Init voxel CPU vectors 
     * Extra position for bounds */
    this->voxelPositions.resize(numVoxels + 1);
    this->voxelData.resize(numVoxels);

    /* Create instanced cube mesh */
    this->cube = Library::createCube();

    /* Voxel positions vbo */
    CHECK_GL_CALL(glBindVertexArray(cube->vaoId));
    CHECK_GL_CALL(glGenBuffers(1, &cubePositionVBO));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, cubePositionVBO));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * voxelPositions.size(), nullptr, GL_DYNAMIC_DRAW));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
    CHECK_GL_CALL(glEnableVertexAttribArray(2));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, cubePositionVBO));
    CHECK_GL_CALL(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));	
    CHECK_GL_CALL(glVertexAttribDivisor(2, 1)); 

    /* Voxel data vbo */
    CHECK_GL_CALL(glGenBuffers(1, &cubeDataVBO));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, cubeDataVBO));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * voxelData.size(), nullptr, GL_DYNAMIC_DRAW));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CHECK_GL_CALL(glEnableVertexAttribArray(3));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, cubeDataVBO));
    CHECK_GL_CALL(glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));	
    CHECK_GL_CALL(glVertexAttribDivisor(3, 1));

    CHECK_GL_CALL(glBindVertexArray(0));
}

/* Visualize voxels */
void VoxelShader::render(const CloudVolume *volume, const glm::mat4 &P, const glm::mat4 &V) {
    updateVoxelData(volume);

    /* Bind projeciton, view matrices */
    loadMatrix(getUniform("P"), &P);
    loadMatrix(getUniform("V"), &V);

    /* Bind mesh */
    CHECK_GL_CALL(glBindVertexArray(cube->vaoId));
    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube->eleBufId));

    /* Reupload voxel positions */
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, cubePositionVBO));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * voxelPositions.size(), &voxelPositions[0], GL_DYNAMIC_DRAW));

    /* Reupload voxel data */
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, cubeDataVBO));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * voxelData.size(), &voxelData[0], GL_DYNAMIC_DRAW));

    /* Individual voxels */
    loadFloat(getUniform("alpha"), alpha);
    loadVector(getUniform("voxelSize"), volume->voxelSize);

    /* Render voxels */
    if (!disableWhite) {
        loadBool(getUniform("isOutline"), false);
        CHECK_GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, (int)cube->eleBuf.size(), GL_UNSIGNED_INT, 0, voxelPositions.size()));
    }

    /* Render voxel outlines and bounds */
    loadBool(getUniform("isOutline"), true);
    CHECK_GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
 
    /* Individual voxels */
    if (!disableWhite && useOutline) {
       CHECK_GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, (int)cube->eleBuf.size(), GL_UNSIGNED_INT, 0, voxelPositions.size()));
    }

    /* Bounds */
    if (!disableBounds) {
        glm::vec3 min(volume->xBounds.x, volume->yBounds.x, volume->zBounds.x);
        glm::vec3 max(volume->xBounds.y, volume->yBounds.y, volume->zBounds.y);
        loadVector(getUniform("voxelSize"), max - min);
        CHECK_GL_CALL(glDrawElementsInstancedBaseInstance(GL_TRIANGLES, (int)cube->eleBuf.size(), GL_UNSIGNED_INT, 0, 1, voxelPositions.size() - 1));
    }

    CHECK_GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    /* Clean up */
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    CHECK_GL_CALL(glBindVertexArray(0));
}

void VoxelShader::updateVoxelData(const CloudVolume *volume) {
    /* Pull volume data out of GPU */
    std::vector<float> buffer(voxelData.size());
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, volume->volId));
    CHECK_GL_CALL(glGetTexImage(GL_TEXTURE_3D, 0, GL_RED, GL_FLOAT, buffer.data()));
    CHECK_GL_CALL(glBindTexture(GL_TEXTURE_3D, 0));

    /* Size of voxels in world-space */
    activeVoxels = 0;
    for (unsigned int i = 0; i < voxelData.size(); i++) {
        /* Update voxel data if data exists */
        float r = buffer[i];
        if (r) {
            activeVoxels++;
            glm::ivec3 voxelIndex = volume->get3DIndices(i);
            voxelPositions[i] = volume->position + volume->reverseVoxelIndex(voxelIndex);
            voxelData[i] = r;
        }
        /* Otherwise reset data */
        else {
            voxelPositions[i].x = 0.f;
            voxelPositions[i].y = 1000000.f; // ensures instanced voxels won't be rendered
            voxelPositions[i].z = 0.f;
            voxelData[i] = 0.f;
        }
    }

    /* Update bounds position */
    glm::vec3 min(volume->xBounds.x, volume->yBounds.x, volume->zBounds.x);
    glm::vec3 max(volume->xBounds.y, volume->yBounds.y, volume->zBounds.y);
    voxelPositions.back() = volume->position + (max + min) / 2.f;
}