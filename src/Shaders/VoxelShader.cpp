#include "VoxelShader.hpp"

#include "Library.hpp"
#include "Camera.hpp"
#include "Model/Mesh.hpp"

#include "glm/gtc/matrix_transform.hpp"

VoxelShader::VoxelShader(int dimension, const std::string &r, const std::string &v, const std::string &f) :
    Shader(r, v, f) {
    int numVoxels = dimension * dimension * dimension;

    /* Init voxel CPU vectors */
    this->voxelPositions.resize(numVoxels);
    this->voxelData.resize(numVoxels);

    /* Create instanced cube mesh */
    this->cube = Library::createCube();

    /* Voxel positions vbo */
    CHECK_GL_CALL(glBindVertexArray(cube->vaoId));
    CHECK_GL_CALL(glGenBuffers(1, &cubePositionVBO));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, cubePositionVBO));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * numVoxels, nullptr, GL_DYNAMIC_DRAW));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
    CHECK_GL_CALL(glEnableVertexAttribArray(2));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, cubePositionVBO));
    CHECK_GL_CALL(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));	
    CHECK_GL_CALL(glVertexAttribDivisor(2, 1)); 

    /* Voxel data vbo */
    CHECK_GL_CALL(glGenBuffers(1, &cubeDataVBO));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, cubeDataVBO));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numVoxels, nullptr, GL_DYNAMIC_DRAW));
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

    /* Render bounds */
    // if (!disableBounds) {
    //     /* Bind mesh */
    //     CHECK_GL_CALL(glBindVertexArray(Library::cube->vaoId));
    //     CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Library::cube->eleBufId));
    //     glm::vec3 min(volume->xBounds.x, volume->yBounds.x, volume->zBounds.x);
    //     glm::vec3 max(volume->xBounds.y, volume->yBounds.y, volume->zBounds.y);
    //     glm::vec3 scale(max - min);
    //     glm::mat4 = glm::mat4(1.f);
    //     M *= glm::translate(glm::mat4(1.f), volume->position + min + scale / 2.f);
    //     M *= glm::scale(glm::mat4(1.f), scale);
    //     loadMatrix(getUniform("M"), &M);
    //     loadBool(getUniform("isOutline"), true);
    //     CHECK_GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    //     CHECK_GL_CALL(glDrawElements(GL_TRIANGLES, (int)Library::cube->eleBuf.size(), GL_UNSIGNED_INT, nullptr));
    //     CHECK_GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    // }

    /* Individual voxels */
    loadFloat(getUniform("alpha"), alpha);
    loadVector(getUniform("voxelSize"), volume->voxelSize);

    /* Bind mesh */
    CHECK_GL_CALL(glBindVertexArray(cube->vaoId));
    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube->eleBufId));

    /* Reupload voxel positions */
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, cubePositionVBO));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * voxelPositions.size(), &voxelPositions[0], GL_DYNAMIC_DRAW));

    /* Reupload voxel data */
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, cubeDataVBO));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * voxelData.size(), &voxelData[0], GL_DYNAMIC_DRAW));

    /* Render voxels */
    loadBool(getUniform("isOutline"), false);
    CHECK_GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, (int)cube->eleBuf.size(), GL_UNSIGNED_INT, 0, voxelPositions.size()));

    /* Render voxel outlines */
    if (useOutline) {
        loadBool(getUniform("isOutline"), true);
        CHECK_GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        CHECK_GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, (int)cube->eleBuf.size(), GL_UNSIGNED_INT, 0, voxelPositions.size()));
        CHECK_GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

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
}