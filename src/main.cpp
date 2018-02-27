#include "Window.hpp"
#include "Camera.hpp"
#include "GLSL.hpp"
#include "Shader.hpp"
#include "Util.hpp"

#include "Mesh.hpp"
#include "Cloud.hpp"

#include "glm/glm.hpp"

#include <vector>

/* Initial values */
float Util::timeStep = 0.f;
int Util::FPS = 0;
double Util::lastFpsTime = 0.0;
double Util::lastFrameTime = 0.0;
double Util::runTime = 0.0;
int Util::nFrames = 0;
std::string RESOURCE_DIR = "../res/";

/* Utility members */
Window window;
Camera camera;

/* Light position */
glm::vec3 lightPos;

/* Cloud billboards */
Shader *billboardShader;
Mesh *quad;
Texture *diffuseTex;
Texture *normalTex;

/* 3D Texture */
Shader *volumeShader;
std::vector<Cloud *> clouds;
GLuint volumeHandle;
#define XBOUNDS glm::vec2(-20, 20)
#define YBOUNDS glm::vec2(-2, 15)
#define ZBOUNDS glm::vec2(-12, 12)
#define VOXELSIZE 64

/* Debug visualization */
Shader *diffuseShader;
Mesh *cube;
std::vector<glm::vec3> cubes;

void createBillboardShader() {
    billboardShader = new Shader(RESOURCE_DIR + "cloud_vert.glsl", RESOURCE_DIR + "cloud_frag.glsl");
    if (!billboardShader->init()) {
        std::cerr << "UNABLE TO INITIALIZE BILLBOARD SHADER" << std::endl;
    }
    
    billboardShader->addAttribute("vertPos");

    billboardShader->addUniform("P");
    billboardShader->addUniform("M");
    billboardShader->addUniform("V");
    billboardShader->addUniform("Vi");

    billboardShader->addUniform("diffuseTex");
    billboardShader->addUniform("normalTex");

    billboardShader->addUniform("lightPos");
}

void createVolumeShader() {
    volumeShader = new Shader(RESOURCE_DIR + "voxelize_vert.glsl", RESOURCE_DIR + "voxelize_frag.glsl");
    if (!volumeShader->init()) {
        std::cerr << "UNABLE TO INITIALIZE VOLUME SHADER" << std::endl;
    }

    volumeShader->addAttribute("vertPos");

    volumeShader->addUniform("P");
    volumeShader->addUniform("V");
    volumeShader->addUniform("M");
    volumeShader->addUniform("Vi");

    volumeShader->addUniform("xBounds");
    volumeShader->addUniform("yBounds");
    volumeShader->addUniform("zBounds");
    volumeShader->addUniform("voxelSize");

    volumeShader->addUniform("volume");
}

void createDiffuseShader() {
    diffuseShader = new Shader(RESOURCE_DIR + "diffuse_vert.glsl", RESOURCE_DIR + "diffuse_frag.glsl");
    if (!diffuseShader->init()) {
        std::cerr << "UNABLE TO INITIALIZE DIFFUSE SHADER" << std::endl;
    }

    diffuseShader->addAttribute("vertPos");
    diffuseShader->addAttribute("vertNor");

    diffuseShader->addUniform("P");
    diffuseShader->addUniform("M");
    diffuseShader->addUniform("V");

    diffuseShader->addUniform("lightPos");
}

void initVolume(int size) {
    glGenTextures(1, &volumeHandle);
    glBindTexture(GL_TEXTURE_3D, volumeHandle);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    //glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, size, size, size, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA16F, size, size, size);
    glClearTexImage(volumeHandle, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void initGeom() {
    /* Create quad mesh */
    quad = new Mesh;
    quad->vertBuf = {
        -1.f, -1.f,  0.f,
         1.f, -1.f,  0.f,
        -1.f,  1.f,  0.f,
         1.f,  1.f,  0.f
    };
    quad->init();

    /* Create cube mesh */
    cube = new Mesh;
    cube->vertBuf = {
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f
    };
    cube->norBuf = {
         0.f, -1.f,  0.f,
         0.f, -1.f,  0.f,
         0.f,  0.f, -1.f,
         0.f,  1.f,  0.f,
         0.f,  1.f,  0.f,
         0.f,  0.f,  1.f,
         0.f,  0.f,  1.f,
         1.f,  0.f,  0.f,
         1.f,  0.f,  0.f,
        -1.f,  0.f,  0.f,
        -1.f,  0.f,  0.f,
    };
    cube->eleBuf = {
		0, 1, 2,
		2, 3, 0,
		1, 5, 6,
		6, 2, 1,
		7, 6, 5,
		5, 4, 7,
		4, 0, 3,
		3, 7, 4,
		4, 5, 1,
		1, 0, 4,
		3, 2, 6,
		6, 7, 3,
    };
    cube->init();
}

void createCloud(glm::vec3 pos) {
    Cloud *cloud = new Cloud;

    cloud->position = pos;
    cloud->size = glm::normalize(glm::vec2(diffuseTex->width, diffuseTex->height));
    cloud->rotation = 0.f;

    clouds.push_back(cloud);
}

void renderCubes() {
    /* Bind shader */
    diffuseShader->bind();
    
    /* Bind projeciton, view, inverise view matrices */
    diffuseShader->loadMat4(diffuseShader->getUniform("P"), &camera.P);
    diffuseShader->loadMat4(diffuseShader->getUniform("V"), &camera.V);

    /* Bind light position */
    diffuseShader->loadVec3(diffuseShader->getUniform("lightPos"), lightPos);

    /* Bind mesh */
    /* VAO */
    glBindVertexArray(cube->vaoId);

    /* Vertices VBO */
    int pos = diffuseShader->getAttribute("vertPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, cube->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    /* Normals VBO */
    pos = diffuseShader->getAttribute("vertNor");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, cube->norBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
 
    /* IBO */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube->eleBufId);

    glm::mat4 M;
    for (auto position : cubes) {
        M  = glm::mat4(1.f);
        M *= glm::scale(glm::mat4(1.f), glm::vec3(1.f));
        M *= glm::translate(glm::mat4(1.f), 5.f*position);
        diffuseShader->loadMat4(diffuseShader->getUniform("M"), &M);

        glDrawElements(GL_TRIANGLES, (int)cube->eleBuf.size(), GL_UNSIGNED_INT, nullptr);
    }

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    diffuseShader->unbind();
}

void renderBillboards() {
    /* Set GL state */
    glDisable(GL_DEPTH_TEST);

    /* Bind billboard shader */
    billboardShader->bind();
    
    /* Bind projeciton, view, inverise view matrices */
    billboardShader->loadMat4(billboardShader->getUniform("P"), &camera.P);
    billboardShader->loadMat4(billboardShader->getUniform("V"), &camera.V);
    glm::mat4 Vi = camera.V;
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    billboardShader->loadMat4(billboardShader->getUniform("Vi"), &Vi);

    /* Bind light position */
    billboardShader->loadVec3(billboardShader->getUniform("lightPos"), lightPos);

    /* Bind mesh */
    /* VAO */
    glBindVertexArray(quad->vaoId);

    /* Vertices VBO */
    int pos = billboardShader->getAttribute("vertPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, quad->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    /* IBO */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad->eleBufId);

    /* Bind textures */
    billboardShader->loadInt(billboardShader->getUniform("diffuseTex"), diffuseTex->textureId);
    glActiveTexture(GL_TEXTURE0 + diffuseTex->textureId);
    glBindTexture(GL_TEXTURE_2D, diffuseTex->textureId);
    billboardShader->loadInt(billboardShader->getUniform("normalTex"), normalTex->textureId);
    glActiveTexture(GL_TEXTURE0 + normalTex->textureId);
    glBindTexture(GL_TEXTURE_2D, normalTex->textureId);

    glm::mat4 M;
    for (auto cloud : clouds) {
        M  = glm::mat4(1.f);
        M *= glm::translate(glm::mat4(1.f), cloud->position);
        // TODO : fix M
        // M *= glm::rotate(glm::mat4(1.f), glm::radians(cloud->rotation), glm::vec3(0, 0, 1));
        // M *= glm::scale(glm::mat4(1.f), glm::vec3(cloud->size, 0.f));
        billboardShader->loadMat4(billboardShader->getUniform("M"), &M);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    billboardShader->unbind();
    glEnable(GL_DEPTH_TEST);
}


void voxelize() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
 
    volumeShader->bind();

    volumeShader->loadMat4(volumeShader->getUniform("P"), &camera.P);
    volumeShader->loadMat4(volumeShader->getUniform("V"), &camera.V);
    glm::mat4 Vi = camera.V;
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    volumeShader->loadMat4(volumeShader->getUniform("Vi"), &Vi);

    glBindImageTexture(1, volumeHandle, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
    volumeShader->loadVec2(volumeShader->getUniform("xBounds"), XBOUNDS);
    volumeShader->loadVec2(volumeShader->getUniform("yBounds"), YBOUNDS);
    volumeShader->loadVec2(volumeShader->getUniform("zBounds"), ZBOUNDS);
    volumeShader->loadInt(volumeShader->getUniform("voxelSize"), VOXELSIZE);
 
    /* Bind quad */
    /* VAO */
    glBindVertexArray(cube->vaoId);

    /* Vertices VBO */
    int pos = volumeShader->getAttribute("vertPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, cube->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    /* IBO */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube->eleBufId);
    
    /* Invoke draw call on a quad - this will write to the 3D texture */
    glm::mat4 M  = glm::mat4(1.f);
    M *= glm::translate(glm::mat4(1.f), glm::vec3(5.f, 0.f, 0.f));
    volumeShader->loadMat4(volumeShader->getUniform("M"), &M);
    //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDrawElements(GL_TRIANGLES, (int)cube->eleBuf.size(), GL_UNSIGNED_INT, nullptr);

    /* Wrap up shader */
    glBindVertexArray(0);
    volumeShader->unbind();

    /* Pull 3D texture out of GPU */
    std::vector<float> buffer(VOXELSIZE * VOXELSIZE * VOXELSIZE * 4);
    glBindTexture(GL_TEXTURE_3D, volumeHandle);
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, buffer.data());

    // TODO : for x, y, z { find index, load billboard at that index }
    for (int i = 0; i < buffer.size(); i+=4) {
        float r = buffer[i + 0];
        float g = buffer[i + 1];
        float b = buffer[i + 2];
        float a = buffer[i + 3];
        if (r || g || b || a) {
            // createCloud(glm::vec3(r, g, b)/255.f);
            cubes.push_back(glm::vec3(r, g, b));
            printf("<%f, %f, %f, %f>\n", (float)r, float(g), float(b), float(a));
        }
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glDepthMask(GL_TRUE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
}

void takeInput() {
    /* Update light */
    if (Keyboard::isKeyPressed(GLFW_KEY_Z)) {
        lightPos.x += 10.f;
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_X)) {
        lightPos.x -= 10.f;
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_C)) {
        lightPos.y += 10.f;
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_V)) {
        lightPos.y -= 10.f;
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_B)) {
        lightPos.z += 10.f;
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_N)) {
        lightPos.z -= 10.f;
    }
}

int main() {
    /* Init window, keyboard, and mouse wrappers */
    if (window.init("Clouds")) {
        std::cerr << "ERROR" << std::endl;
        return 1;
    }

    /* Create light */
    lightPos = glm::vec3(100.f, 100.f, 100.f);

    /* Create shaders */
    createBillboardShader();
    createVolumeShader();
    createDiffuseShader();

    /* Create meshes */
    initGeom();
    initVolume(VOXELSIZE);

    /* Create textures */
    diffuseTex = new Texture(RESOURCE_DIR + "cloud.png");
    normalTex = new Texture(RESOURCE_DIR + "cloudmap.png");

    /* Init rendering */
    GLSL::checkVersion();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* First render pass - generate volume */
    Util::updateTiming(glfwGetTime());
    window.update();
    camera.update(Util::timeStep);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.f, 0.4f, 0.7f, 1.f);
    /* Generate 3D Volume*/
    voxelize();
    while (!window.shouldClose()) {
        /* Update context */
        Util::updateTiming(glfwGetTime());
        window.update();

        /* Update camera */
        camera.update(Util::timeStep);

        takeInput();

        /* Render */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.f, 0.4f, 0.7f, 1.f);
        renderBillboards();
        renderCubes();
    }
}
