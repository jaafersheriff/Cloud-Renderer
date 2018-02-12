#include "Window.hpp"
#include "Camera.hpp"
#include "GLSL.hpp"
#include "Shader.hpp"
#include "Util.hpp"

#include "Mesh.hpp"
#include "Cloud.hpp"

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

/* Shaders */
Shader *billboardShader;
Shader *volumeShader;

/* Render targets */
std::vector<Cloud *> clouds;
GLuint volumeHandle;
#define VOLUMESIZE 128

/* Utility render targets */
Mesh *quad;
Texture *diffuseTex;
Texture *normalTex;

void createBillboardShader() {
    billboardShader = new Shader(RESOURCE_DIR + "cloud_vert.glsl", RESOURCE_DIR + "cloud_frag.glsl");
    billboardShader->init();

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
    volumeShader->init();

    volumeShader->addAttribute("vertPos");
    volumeShader->addAttribute("vertTex");

    volumeShader->addUniform("P");
    volumeShader->addUniform("V");
    volumeShader->addUniform("M");
    volumeShader->addUniform("Vi");

    volumeShader->addUniform("volume");
}

void createClouds() {
    /* Create quad mesh */
    quad = new Mesh;
    quad->vertBuf = {
        -1.f, -1.f,  0.f,
         1.f, -1.f,  0.f,
        -1.f,  1.f,  0.f,
         1.f,  1.f,  0.f
    };
    quad->init();

    /* Create textures */
    diffuseTex = new Texture(RESOURCE_DIR + "cloud.png");
    normalTex = new Texture(RESOURCE_DIR + "cloudmap.png");

    for (int i = 0; i < 30; i++) {
        Cloud *cloud = new Cloud;

        cloud->position = Util::genRandomVec3(-1.f, 1.f);
        cloud->position.x += 10.f;
        cloud->size = glm::normalize(glm::vec2(diffuseTex->width, diffuseTex->height));
        cloud->rotation = 0.f;

        clouds.push_back(cloud);
    }
}

void createVolume(int size) {
    glGenTextures(1, &volumeHandle);
    glBindTexture(GL_TEXTURE_3D, volumeHandle);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA16F, size, size, size, 0, GL_RGBA, GL_FLOAT, nullptr);
    glClearTexImage(volumeHandle, 0, GL_RGBA, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void renderBillboards() {
    /* Set GL state */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
}


void voxelize() {
    /* Generate 3D texture */
    volumeShader->bind();
    volumeShader->loadMat4(volumeShader->getUniform("P"), &camera.P);
    volumeShader->loadMat4(volumeShader->getUniform("V"), &camera.V);
    glm::mat4 Vi = camera.V;
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    volumeShader->loadMat4(volumeShader->getUniform("Vi"), &Vi);
    volumeShader->loadInt(volumeShader->getUniform("volume"), volumeHandle);
    glActiveTexture(GL_TEXTURE0 + volumeHandle);
    glBindImageTexture(1, volumeHandle, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA16F);
 
    /* Bind quad */
    /* VAO */
    glBindVertexArray(quad->vaoId);

    /* Vertices VBO */
    int pos = volumeShader->getAttribute("vertPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, quad->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    /* IBO */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad->eleBufId);

    /* Draw quad */
    glm::mat4 M  = glm::mat4(1.f);
    volumeShader->loadMat4(volumeShader->getUniform("M"), &M);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    /* Wrap up shader */
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
    volumeShader->unbind();

    /* Pull 3D texture out of GPU */
    std::vector<unsigned char> buffer(VOLUMESIZE * VOLUMESIZE * VOLUMESIZE * 4);
    glBindTexture(GL_TEXTURE_3D, volumeHandle);
    glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
    for (int i = 0; i < buffer.size(); i+=4) {
        BYTE red = buffer[i];
        if (red > 0) {
            int z = 0;
        }
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

    /* Create clouds and volume */
    createClouds();
    createVolume(VOLUMESIZE);

    /* Init rendering */
    GLSL::checkVersion();
    glClearColor(0.f, 0.8f, 0.9f, 1.f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    while (!window.shouldClose()) {
        /* Update context */
        Util::updateTiming(glfwGetTime());
        window.update();

        /* Update camera */
        camera.update(Util::timeStep);

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

        /* Render */
        voxelize();
        renderBillboards();
    }
}
