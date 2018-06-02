#include "VoxelShader.hpp"

#include "Library.hpp"
#include "Light.hpp"
#include "Camera.hpp"
#include "Model/Mesh.hpp"

#include "glm/gtc/matrix_transform.hpp"

/* Visualize voxels */
void VoxelShader::render(CloudVolume *volume, glm::mat4 P, glm::mat4 V) {
    /* Bind projeciton, view, inverise view matrices */
    loadMatrix(getUniform("P"), &P);
    loadMatrix(getUniform("V"), &V);
    loadFloat(getUniform("alpha"), alpha);

    /* Bind mesh */
    CHECK_GL_CALL(glBindVertexArray(Library::cube->vaoId));
    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Library::cube->eleBufId));

    glm::mat4 M;
    
    /* Render bounds */
    if (!disableBounds) {
        glm::vec3 min(volume->xBounds.x, volume->yBounds.x, volume->zBounds.x);
        glm::vec3 max(volume->xBounds.y, volume->yBounds.y, volume->zBounds.y);
        glm::vec3 scale(max - min);
        M = glm::mat4(1.f);
        M *= glm::translate(glm::mat4(1.f), volume->position + min + scale / 2.f);
        M *= glm::scale(glm::mat4(1.f), scale);
        loadMatrix(getUniform("M"), &M);
        loadBool(getUniform("isOutline"), true);
        CHECK_GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
        CHECK_GL_CALL(glDrawElements(GL_TRIANGLES, (int)Library::cube->eleBuf.size(), GL_UNSIGNED_INT, nullptr));
        CHECK_GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    /* Render individual voxels */
    for (auto v : volume->voxelData) {
        if (v.data.r || v.data.g || v.data.b || v.data.a) {
            if ((disableBlack && glm::vec3(v.data) == glm::vec3(0.f)) ||
                (disableWhite && v.data == glm::vec4(1.f))) {
                continue;
            }
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