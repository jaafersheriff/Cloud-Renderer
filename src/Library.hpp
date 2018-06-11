#pragma once
#ifndef _LIBRARY_HPP_
#define _LIBRARY_HPP_

#include "Model/Mesh.hpp"
#include "Model/Texture.hpp"

#include <map>

class Library {
public:
    static Mesh * quad;
    static Mesh * quadInstanced;
    static GLuint quadInstancedPositionVBO;
    static GLuint quadInstancedScaleVBO;
    static std::map<std::string, Mesh *> meshes;
    static std::map<std::string, Texture *> textures;

    static void init(int count) {
        /* Create meshes */
        createQuad(&quad);
        createQuad(&quadInstanced);
        glBindVertexArray(quadInstanced->vaoId);
        glGenBuffers(1, &quadInstancedPositionVBO);
        glBindBuffer(GL_ARRAY_BUFFER, quadInstancedPositionVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * count, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, quadInstancedPositionVBO);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);	
        glVertexAttribDivisor(2, 1);  
        glGenBuffers(1, &quadInstancedScaleVBO);
        glBindBuffer(GL_ARRAY_BUFFER, quadInstancedScaleVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * count, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glEnableVertexAttribArray(3);
        glBindBuffer(GL_ARRAY_BUFFER, quadInstancedScaleVBO);
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);	
        glVertexAttribDivisor(3, 1);  
    }

    static void addTexture(std::string res, std::string fileName) {
        Texture *texture = new Texture(res + fileName);
        if (texture->textureId) {
            textures[fileName] = texture;
        }
        else {
            delete texture;
        }
    }

    static Mesh * createCube() {
        Mesh *m = new Mesh;
        m->vertBuf = {
            -0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f, -0.5f,
            -0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,
             0.5f,  0.5f, -0.5f,
            -0.5f,  0.5f,  0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f,  0.5f, -0.5f,
             0.5f,  0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f, -0.5f,
             0.5f, -0.5f, -0.5f,
             0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
            -0.5f, -0.5f,  0.5f,
             0.5f, -0.5f,  0.5f,
             0.5f,  0.5f,  0.5f,
            -0.5f,  0.5f,  0.5f
        };
        m->norBuf = {
             0,  0, -1,
             0,  0, -1,
             0,  0, -1,
             0,  0, -1,
            -1,  0,  0,
            -1,  0,  0,
            -1,  0,  0,
            -1,  0,  0,
             0,  1,  0,
             0,  1,  0,
             0,  1,  0,
             0,  1,  0,
             1,  0,  0,
             1,  0,  0,
             1,  0,  0,
             1,  0,  0,
             0, -1,  0,
             0, -1,  0,
             0, -1,  0,
             0, -1,  0,
             0,  0,  1,
             0,  0,  1,
             0,  0,  1,
             0,  0,  1,
        };
        m->eleBuf = {
             0,  1,  2,
             0,  3,  1,
             4,  5,  6,
             4,  7,  5,
             8,  9, 10,
             8, 11,  9,
            12, 13, 14,
            12, 14, 15,
            16, 17, 18,
            16, 18, 19,
            20, 21, 22,
            20, 22, 23,
        };
        m->init();
        return m;
    }
    static void createQuad(Mesh **mesh) {
        Mesh *m = new Mesh;
        *mesh = m;
        m->vertBuf = {
            -1.f, -1.f,  0.f,
             1.f, -1.f,  0.f,
            -1.f,  1.f,  0.f,
             1.f,  1.f,  0.f
        };
        m->norBuf = {
            0.f, 0.f, 1.f,
            0.f, 0.f, 1.f,
            0.f, 0.f, 1.f,
            0.f, 0.f, 1.f
        };
        m->init();
    }
};

#endif