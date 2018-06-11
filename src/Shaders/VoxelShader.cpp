#include "VoxelShader.hpp"

#include "Library.hpp"
#include "Camera.hpp"
#include "Model/Mesh.hpp"

#include "glm/gtc/matrix_transform.hpp"

VoxelShader::VoxelShader(int count, const std::string &r, const std::string &v, const std::string &f) :
    Shader(r, v, f) {
    this->cube = Library::createCube();
    CHECK_GL_CALL(glBindVertexArray(cube->vaoId));
    CHECK_GL_CALL(glGenBuffers(1, &cubePositionVBO));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, cubePositionVBO));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * count, nullptr, GL_DYNAMIC_DRAW));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0)); 
    CHECK_GL_CALL(glEnableVertexAttribArray(2));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, cubePositionVBO));
    CHECK_GL_CALL(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));	
    CHECK_GL_CALL(glVertexAttribDivisor(2, 1)); 
    CHECK_GL_CALL(glGenBuffers(1, &cubeDataVBO));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, cubeDataVBO));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * count, nullptr, GL_DYNAMIC_DRAW));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CHECK_GL_CALL(glEnableVertexAttribArray(3));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, cubeDataVBO));
    CHECK_GL_CALL(glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));	
    CHECK_GL_CALL(glVertexAttribDivisor(3, 1));
}

/* Visualize voxels */
void VoxelShader::render(const CloudVolume *volume, const glm::mat4 &P, const glm::mat4 &V) {
    /* Bind projeciton, view, inverise view matrices */
    loadMatrix(getUniform("P"), &P);
    loadMatrix(getUniform("V"), &V);
    loadFloat(getUniform("alpha"), alpha);

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

    /* Render individual voxels */
    loadVector(getUniform("voxelSize"), volume->voxelSize);

    /* Bind mesh */
    CHECK_GL_CALL(glBindVertexArray(cube->vaoId));
    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube->eleBufId));

    glBindBuffer(GL_ARRAY_BUFFER, cubePositionVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * volume->voxelPositions.size(), &volume->voxelPositions[0], GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, cubeDataVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * volume->voxelData.size(), &volume->voxelData[0], GL_DYNAMIC_DRAW);

    loadBool(getUniform("isOutline"), false);
    CHECK_GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, (int)cube->eleBuf.size(), GL_UNSIGNED_INT, 0, volume->voxelPositions.size()));

    loadBool(getUniform("isOutline"), true);
    CHECK_GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    CHECK_GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, (int)cube->eleBuf.size(), GL_UNSIGNED_INT, 0, volume->voxelPositions.size()));
    CHECK_GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    CHECK_GL_CALL(glBindVertexArray(0));
}