#include "VoxelShader.hpp"

#include "Library.hpp"
#include "Light.hpp"
#include "Camera.hpp"
#include "Model/Mesh.hpp"

#include "glm/gtc/matrix_transform.hpp"

VoxelShader::VoxelShader(std::string v, std::string f) :
    Shader(v, f) {
    init();
}

/* Visualize voxels */
void VoxelShader::render(std::vector<Volume::Voxel> &voxels, glm::mat4 P, glm::mat4 V) {
    /* Bind projeciton, view, inverise view matrices */
    loadMatrix(getUniform("P"), &P);
    loadMatrix(getUniform("V"), &V);
    loadFloat(getUniform("alpha"), alpha);

    /* Bind mesh */
    CHECK_GL_CALL(glBindVertexArray(Library::cube->vaoId));
    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Library::cube->eleBufId));

    glm::mat4 M;
    for (auto v : voxels) {
        if (v.data.r || v.data.g || v.data.b || v.data.a) {
            M = glm::mat4(1.f);
            M *= glm::translate(glm::mat4(1.f), v.spatial.position);
            M *= glm::scale(glm::mat4(1.f), v.spatial.scale);
            loadMatrix(getUniform("M"), &M);

            loadVector(getUniform("voxelData"), v.data);

            /* Draw shape */
            loadBool(getUniform("isOutline"), false);
            CHECK_GL_CALL(glDrawElements(GL_TRIANGLES, (int)Library::cube->eleBuf.size(), GL_UNSIGNED_INT, nullptr));

            /* Draw outline */
            if (useOutline) {
                loadBool(getUniform("isOutline"), true);
                CHECK_GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
                CHECK_GL_CALL(glDrawElements(GL_TRIANGLES, (int)Library::cube->eleBuf.size(), GL_UNSIGNED_INT, nullptr));
                CHECK_GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
            }
        }
    }

    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    CHECK_GL_CALL(glBindVertexArray(0));
}