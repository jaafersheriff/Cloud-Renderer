#include "IO/Window.hpp"
#include "Camera.hpp"
#include "GL/GLSL.hpp"
#include "GL/Shader.hpp"
#include "Util.hpp"

#include "Model/Mesh.hpp"
#include "Cloud.hpp"

#include <vector>

std::string RESOURCE_DIR = "../res/";

Window window;
Camera camera;

std::vector<Cloud *> clouds;
Shader *cloudShader;

Mesh *quad;
Texture *diffuseTex;
Texture *normalTex;

/* Timing */
double lastFpsTime, lastFrameTime, runTime;
float timeStep;
int nFrames, FPS;
void updateTiming() {
    runTime = (float)window.getTime();
    timeStep = (float) runTime - lastFrameTime;
    lastFrameTime = runTime;
    nFrames++;
    if (runTime - lastFpsTime >= 1.0) {
        FPS = nFrames;
        nFrames = 0;
        lastFpsTime = runTime;
    }
}

void takeInput() {
    if (Mouse::isDown(0)) {
        camera.takeMouseInput(Mouse::dx, Mouse::dy);
    }
    else {
        camera.takeMouseInput(0.0, 0.0);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_W)) {
        camera.moveForward(timeStep);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_S)) {
        camera.moveBackward(timeStep);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_A)) {
        camera.moveLeft(timeStep);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_D)) {
        camera.moveRight(timeStep);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_R)) {
        camera.moveUp(timeStep);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_E)) {
        camera.moveDown(timeStep);
    }   
}

void createShader() {
    /* Create shader */
    cloudShader = new Shader(RESOURCE_DIR + "cloud_vert.glsl", RESOURCE_DIR + "cloud_frag.glsl");
    cloudShader->init();

    cloudShader->addAttribute("vertPos");

    cloudShader->addUniform("P");
    cloudShader->addUniform("M");
    cloudShader->addUniform("V");

    cloudShader->addUniform("center");
    cloudShader->addUniform("size");

    cloudShader->addUniform("diffuseTex");
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
    diffuseTex = new Texture(RESOURCE_DIR + "smoke.png");
    normalTex = new Texture(RESOURCE_DIR + "NormalMap.png");

    for (int i = 0; i < 30; i++) {
        Cloud *cloud = new Cloud;

        cloud->position = Util::genRandomVec3(-5.f, 5.f);
        cloud->size = glm::normalize(glm::vec2(diffuseTex->width, diffuseTex->height));
        cloud->rotation = 0.f;

        clouds.push_back(cloud);
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    cloudShader->bind();

    cloudShader->loadMat4(cloudShader->getUniform("P"), &camera.P);
    cloudShader->loadMat4(cloudShader->getUniform("V"), &camera.V);
        
    /* Bind mesh */
    /* VAO */
    glBindVertexArray(quad->vaoId);

    /* Vertices VBO */
    int pos = cloudShader->getAttribute("vertPos");
    glEnableVertexAttribArray(pos);
    glBindBuffer(GL_ARRAY_BUFFER, quad->vertBufId);
    glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

    /* IBO */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad->eleBufId);

    /* Bind textures */
    cloudShader->loadInt(cloudShader->getUniform("diffuseTex"), diffuseTex->textureId);
    glActiveTexture(GL_TEXTURE0 + diffuseTex->textureId);
    glBindTexture(GL_TEXTURE_2D, diffuseTex->textureId);

    for (auto cloud : clouds) {
        cloudShader->loadVec3(cloudShader->getUniform("center"), cloud->position);
        cloudShader->loadVec2(cloudShader->getUniform("size"), cloud->size);

        glm::mat4 M = glm::rotate(glm::mat4(1.f), glm::radians(cloud->rotation), glm::vec3(0, 0, 1));
        cloudShader->loadMat4(cloudShader->getUniform("M"), &M);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glBindVertexArray(0);
    cloudShader->unbind();
}

int main() {
    /* Timing */
    timeStep = lastFpsTime = lastFrameTime = runTime = 0.0;
    nFrames = FPS = 0;

    /* Init window, keyboard, and mouse wrappers */
    window.init("Clouds");

    /* Create cloud shader */
    createShader();

    /* Create clouds*/
    createClouds();

    /* Init rendering */
    GLSL::checkVersion();
    glClearColor(0.f, 0.8f, 0.9f, 1.f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    while (!window.shouldClose()) {
        /* Update context */
        updateTiming();
        window.update();

        /* Update camera */
        takeInput();
        camera.update();

        /* Render clouds*/
        render();
    }
}
