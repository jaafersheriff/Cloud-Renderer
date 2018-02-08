#include "IO/Window.hpp"
#include "Camera.hpp"
#include "GL/GLSL.hpp"
#include "GL/Shader.hpp"
#include "Util.hpp"

#include "Model/Mesh.hpp"
#include "Cloud.hpp"

#include <vector>

float Util::timeStep = 0.f;
int Util::FPS = 0;
double Util::lastFpsTime = 0.0;
double Util::lastFrameTime = 0.0;
double Util::runTime = 0.0;
int Util::nFrames = 0;

std::string RESOURCE_DIR = "../res/";

Window window;
Camera camera;

glm::vec3 lightPos;

std::vector<Cloud *> clouds;
Shader *cloudShader;

Mesh *quad;
Texture *diffuseTex;
Texture *normalTex;

void createShader() {
    /* Create shader */
    cloudShader = new Shader(RESOURCE_DIR + "cloud_vert.glsl", RESOURCE_DIR + "cloud_frag.glsl");
    cloudShader->init();

    cloudShader->addAttribute("vertPos");

    cloudShader->addUniform("P");
    cloudShader->addUniform("M");
    cloudShader->addUniform("V");
    cloudShader->addUniform("Vi");

    cloudShader->addUniform("diffuseTex");
    cloudShader->addUniform("normalTex");

    cloudShader->addUniform("lightPos");
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
        cloud->size = glm::normalize(glm::vec2(diffuseTex->width, diffuseTex->height));
        cloud->rotation = 0.f;

        clouds.push_back(cloud);
    }
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);

    cloudShader->bind();
    
    /* Bind projeciton, view, inverise view matrices */
    cloudShader->loadMat4(cloudShader->getUniform("P"), &camera.P);
    cloudShader->loadMat4(cloudShader->getUniform("V"), &camera.V);
    glm::mat4 Vi = camera.V;
    Vi[3][0] = Vi[3][1] = Vi[3][2] = 0.f;
    Vi = glm::transpose(Vi);
    cloudShader->loadMat4(cloudShader->getUniform("Vi"), &Vi);

    /* Bind light position */
    cloudShader->loadVec3(cloudShader->getUniform("lightPos"), lightPos);

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
    cloudShader->loadInt(cloudShader->getUniform("normalTex"), normalTex->textureId);
    glActiveTexture(GL_TEXTURE0 + diffuseTex->textureId);
    glBindTexture(GL_TEXTURE_2D, diffuseTex->textureId);
    glActiveTexture(GL_TEXTURE0 + normalTex->textureId);
    glBindTexture(GL_TEXTURE_2D, normalTex->textureId);

    glm::mat4 M;
    for (auto cloud : clouds) {
        M  = glm::mat4(1.f);
        M *= glm::translate(glm::mat4(1.f), cloud->position);
        // M *= glm::rotate(glm::mat4(1.f), glm::radians(cloud->rotation), glm::vec3(0, 0, 1));
        // M *= glm::scale(glm::mat4(1.f), glm::vec3(cloud->size, 0.f));
        cloudShader->loadMat4(cloudShader->getUniform("M"), &M);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glBindVertexArray(0);
    cloudShader->unbind();
}

void updateLight() {
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
    window.init("Clouds");

    /* Create light */
    lightPos = glm::vec3(100.f, 100.f, 100.f);

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
        Util::updateTiming(glfwGetTime());
        window.update();

        /* Update camera */
        camera.update(Util::timeStep);

        /* Update light */
        updateLight();

        /* Render clouds*/
        render();
    }
}
