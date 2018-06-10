#pragma once
#ifndef _LIBRARY_HPP_
#define _LIBRARY_HPP_

#include "Model/Mesh.hpp"
#include "Model/Texture.hpp"

#include <map>

class Library {
public:
    static Mesh * cube;
    static Mesh * cubeInstanced;
    static GLuint cubeInstancedPositionVBO;
    static GLuint cubeInstancedDataVBO;
    static Mesh * quad;
    static std::map<std::string, Mesh *> meshes;
    static std::map<std::string, Texture *> textures;

    static void init(int count) {
        /* Create meshes */
        createCube(&cube);
        createQuad();

        createCube(&cubeInstanced);
        glBindVertexArray(cubeInstanced->vaoId);
        // positions vbo
        glGenBuffers(1, &cubeInstancedPositionVBO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeInstancedPositionVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * count, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, cubeInstancedPositionVBO);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);	
        glVertexAttribDivisor(2, 1);  

        // data vbo
        glGenBuffers(1, &cubeInstancedDataVBO);
        glBindBuffer(GL_ARRAY_BUFFER, cubeInstancedDataVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * count, nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0); 
        glEnableVertexAttribArray(3);
        glBindBuffer(GL_ARRAY_BUFFER, cubeInstancedDataVBO);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
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

    static void createCube(Mesh **mesh) {
        Mesh *m = new Mesh;
        *mesh = m;
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
    }
    static void createQuad() {
        quad = new Mesh;
        quad->vertBuf = {
            -1.f, -1.f,  0.f,
             1.f, -1.f,  0.f,
            -1.f,  1.f,  0.f,
             1.f,  1.f,  0.f
        };
        quad->norBuf = {
            0.f, 0.f, 1.f,
            0.f, 0.f, 1.f,
            0.f, 0.f, 1.f,
            0.f, 0.f, 1.f
        };
        quad->init();
    }
};

#endif