#pragma once
#ifndef _LIBRARY_HPP_
#define _LIBRARY_HPP_

#include "Model/Mesh.hpp"
#include "Model/Texture.hpp"

class Library {
    public:
        static Mesh * cube;
        static Mesh * quad;
        static Texture * cloudDiffuseTexture;
        static Texture * cloudNormalTexture;

        static void init(std::string diffuseName, std::string normalName) {
            /* Create meshes */
            createCube();
            createQuad();

            /* Create textures */
            cloudDiffuseTexture = new Texture(diffuseName);
            cloudNormalTexture = new Texture(normalName);
        }

        static void createCube() {
            cube = new Mesh;
            cube->vertBuf = {
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
            cube->norBuf = {
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
            cube->eleBuf = {
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
            cube->init();
        }
        static void createQuad() {
            quad = new Mesh;
            quad->vertBuf = {
                -0.5f, -0.5f,  0.f,
                 0.5f, -0.5f,  0.f,
                -0.5f,  0.5f,  0.f,
                 0.5f,  0.5f,  0.f
            };
            quad->init();
        }
};

#endif