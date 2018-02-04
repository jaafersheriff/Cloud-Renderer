#include "Window.hpp"
#include "Camera.hpp"
#include "GLSL.hpp"
#include "Shader.hpp"
#include "Util.hpp"

#include "Cloud.hpp"

std::string RESOURCE_DIR = "../res/";

Window window;
Camera camera;

std::vector<Cloud *> clouds;
Shader *cloudShader;

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
    if (Keyboard::isKeyPressed(GLFW_KEY_E)) {
        camera.moveUp(timeStep);
    }
    if (Keyboard::isKeyPressed(GLFW_KEY_R)) {
        camera.moveDown(timeStep);
    }   
}

void createShader() {
    /* Create shader */
    cloudShader = new Shader(RESOURCE_DIR + "cloud_vert.glsl", RESOURCE_DIR + "cloud_frag.glsl");
    cloudShader->init();
    cloudShader->addUniform("P");
    cloudShader->addUniform("M");
    cloudShader->addUniform("V");
    cloudShader->addAttribute("vertPos");
    cloudShader->addAttribute("vertTex");
}

void createClouds() {
    /* Create textures */
    Texture *cloudTexture = new Texture(RESOURCE_DIR + "smoke.png");
    Texture *normalMap = new Texture(RESOURCE_DIR + "NormalMap.png");

    for (int i = 0; i < 30; i++) {
        Cloud *cloud = new Cloud;

        cloud->position =  Util::genRandomVec3(-5.f, 5.f);
        cloud->rotation =  Util::genRandomVec3(0.f, 360.f);
        cloud->scale = glm::vec3(10.f);

        cloud->texture = cloudTexture;
        cloud->normalMap = normalMap;
        clouds.push_back(cloud);
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cloudShader->bind();

    cloudShader->loadMat4(cloudShader->getUniform("P"), &camera.P);
    cloudShader->loadMat4(cloudShader->getUniform("V"), &camera.V);


    glm::mat4 M;
    for (auto cloud : clouds) {
        glBindVertexArray(clouds[0]->mesh->vaoId);
        int pos = cloudShader->getAttribute("vertPos");
        glEnableVertexAttribArray(pos);
        glBindBuffer(GL_ARRAY_BUFFER, cloud->mesh->vertBufId);
        glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        pos = cloudShader->getAttribute("vertTex");
        glEnableVertexAttribArray(pos);
        glBindBuffer(GL_ARRAY_BUFFER, cloud->mesh->texBufId);
        glVertexAttribPointer(pos, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, clouds[0]->mesh->eleBufId);
        M = glm::mat4(1.f);
        M *= glm::translate(glm::mat4(1.f), cloud->position);
        M *= glm::rotate(glm::mat4(1.f), glm::radians(cloud->rotation.x), glm::vec3(1, 0, 0));
        M *= glm::rotate(glm::mat4(1.f), glm::radians(cloud->rotation.y), glm::vec3(0, 1, 0));
        M *= glm::rotate(glm::mat4(1.f), glm::radians(cloud->rotation.z), glm::vec3(0, 0, 1));
        M *= glm::scale(glm::mat4(1.f), cloud->scale);
        cloudShader->loadMat4(cloudShader->getUniform("M"), &M);

        glDrawElements(GL_TRIANGLES, (int)cloud->mesh->eleBuf.size(), GL_UNSIGNED_INT, nullptr);
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
    glClearColor(0.f, 0.f, 0.1f, 1.f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    while (!window.shouldClose()) {
        updateTiming();
        window.update();
        takeInput();
        camera.update();
        render();
    }
}
