#pragma once
#ifndef _LIBRARY_HPP_
#define _LIBRARY_HPP_

#include "Model/Mesh.hpp"
#include "Model/Texture.hpp"

#include "ThirdParty/tiny_obj_loader.h"

#include <map>

class Library {
public:
    static Mesh * cube;
    static Mesh * quad;
    static std::map<std::string, Mesh *> meshes;
    static std::map<std::string, Texture *> textures;

    static void init() {
        /* Create meshes */
        createCube();
        createQuad();
    }

    static Mesh * addMesh(std::string res, std::string fileName) {
        std::map<std::string, Mesh*>::iterator it = meshes.find(fileName);
        if (it != meshes.end()) {
            return it->second;
        }

        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> objMaterials;
        std::string errString;
        bool rc = tinyobj::LoadObj(shapes, objMaterials, errString, (RESOURCE_DIR + fileName).c_str());
        if (!rc) {
            std::cerr << errString << std::endl;
            exit(1);
        }

        /* Create a new empty mesh */
        Mesh *mesh = new Mesh;
        int vertCount = 0;
        /* For every shape in the loaded file */
        for (unsigned int i = 0; i < shapes.size(); i++) {
            /* Concatenate the shape's vertices, normals, and textures to the mesh */
            mesh->vertBuf.insert(mesh->vertBuf.end(), shapes[i].mesh.positions.begin(), shapes[i].mesh.positions.end());
            mesh->norBuf.insert(mesh->norBuf.end(), shapes[i].mesh.normals.begin(), shapes[i].mesh.normals.end());
            mesh->texBuf.insert(mesh->texBuf.end(), shapes[i].mesh.texcoords.begin(), shapes[i].mesh.texcoords.end());

            /* Concatenate the shape's indices to the new mesh
             * Indices need to be incremented as we concatenate shapes */
            for (unsigned int i : shapes[i].mesh.indices) {
                mesh->eleBuf.push_back(i + vertCount);
            }
            vertCount += shapes[i].mesh.positions.size() / 3;
        }

        /* Copy mesh data to the gpu */
        mesh->init();

        meshes.insert(std::map<std::string, Mesh*>::value_type(fileName, mesh));

        std::cout << "Loaded mesh (" << vertCount << " vertices): " << fileName << std::endl;

        return mesh;
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