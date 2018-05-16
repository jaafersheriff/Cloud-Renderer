#include "Mesh.hpp"
#include "Shaders/GLSL.hpp"

#include <glad/glad.h>
#include <cassert>

/* Constructor */
Mesh::Mesh() :
    vaoId(0),
    vertBufId(0),
    norBufId(0),
    texBufId(0),
    eleBufId(0)
{
}

void Mesh::init() {

    /* Initialize VAO */
    CHECK_GL_CALL(glGenVertexArrays(1, &vaoId));
    CHECK_GL_CALL(glBindVertexArray(vaoId));

    /* Copy vertex array */
    CHECK_GL_CALL(glGenBuffers(1, &vertBufId));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vertBufId));
    CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, vertBuf.size() * sizeof(float), &vertBuf[0], GL_DYNAMIC_DRAW));
    CHECK_GL_CALL(glEnableVertexAttribArray(0));
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, vertBufId));
    CHECK_GL_CALL(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr));

    /* Copy normal array if it exists */
    if (!norBuf.empty()) {
        CHECK_GL_CALL(glGenBuffers(1, &norBufId));
        CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufId));
        CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, norBuf.size() * sizeof(float), &norBuf[0], GL_DYNAMIC_DRAW));
        CHECK_GL_CALL(glEnableVertexAttribArray(1));
        CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, norBufId));
        CHECK_GL_CALL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr));
    }

    /* Copy texture array if it exists */
    if (!texBuf.empty()) {
        CHECK_GL_CALL(glGenBuffers(1, &texBufId));
        CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufId));
        CHECK_GL_CALL(glBufferData(GL_ARRAY_BUFFER, texBuf.size() * sizeof(float), &texBuf[0], GL_DYNAMIC_DRAW));
        CHECK_GL_CALL(glEnableVertexAttribArray(2));
        CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, texBufId));
        CHECK_GL_CALL(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, nullptr));
    }

    /* Copy element array if it exists */
    if (!eleBuf.empty()) {
        CHECK_GL_CALL(glGenBuffers(1, &eleBufId));
        CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eleBufId));
        CHECK_GL_CALL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, eleBuf.size() * sizeof(unsigned int), &eleBuf[0], GL_DYNAMIC_DRAW));
    }

    /* Unbind  */
    CHECK_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    CHECK_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    CHECK_GL_CALL(glBindVertexArray(0));
}