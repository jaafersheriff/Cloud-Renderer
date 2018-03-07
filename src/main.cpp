#include "IO/Window.hpp"
#include "Camera.hpp"
#include "Util.hpp"

#include "Shaders/GLSL.hpp"
#include "Shaders/BillboardShader.hpp"
#include "Shaders/VolumeShader.hpp"
#include "Shaders/DiffuseShader.hpp"

#include "Cloud.hpp"

#include "ThirdParty/imgui/imgui.h"

#include <functional>
#include <time.h>

/* Initial values */
std::string RESOURCE_DIR = "../res/";

/* Light position */
glm::vec3 lightPos;

/* Shaders */
BillboardShader *billboardShader;
VolumeShader *volumeShader;
DiffuseShader *diffuseShader;

/* Volume quad */
glm::vec3 volumePos(5.f, 0.f, 0.f);
glm::vec3 volumeScale(8.f);

/* Geometry */
Mesh *quad;
Mesh *cube;

/* ImGui functions */
std::vector<std::function<void()>> imGuiFuncs;

void initGeom();
void createImGuiPanes();
int main() {
    srand((unsigned int)(time(0)));  

    /* Init window, keyboard, and mouse wrappers */
    if (Window::init("Clouds", 13.f)) {
        std::cerr << "ERROR" << std::endl;
        return 1;
    }

    /* Create quad and cube */
    initGeom();

    /* Create light */
    lightPos = glm::vec3(100.f, 100.f, 100.f);

    /* Create shaders */
    billboardShader = new BillboardShader(RESOURCE_DIR + "cloud_vert.glsl", RESOURCE_DIR + "cloud_frag.glsl");
    billboardShader->init(RESOURCE_DIR + "cloud.png", RESOURCE_DIR + "cloudMap.png", quad);
    volumeShader = new VolumeShader(RESOURCE_DIR + "voxelize_vert.glsl", RESOURCE_DIR + "voxelize_frag.glsl");
    volumeShader->init(32, glm::vec2(-16.f, 16.f), glm::vec2(-16.f, 16.f), glm::vec2(-16.f, 16.f));
    diffuseShader = new DiffuseShader(RESOURCE_DIR + "diffuse_vert.glsl", RESOURCE_DIR + "diffuse_frag.glsl");
    diffuseShader->init();

    /* Init ImGui Panes */
    createImGuiPanes();

    /* Init rendering */
    GLSL::checkVersion();
    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glEnable(GL_BLEND));
    CHECK_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    Camera::update(Window::timeStep);

    while (!Window::shouldClose()) {
        /* Update context */
        Window::update();

        /* Update camera */
        Camera::update(Window::timeStep);

        /* IMGUI */
        if (Window::isImGuiEnabled()) {
            for (auto func : imGuiFuncs) {
                func();
            }
        }

        /* Render */
        CHECK_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        CHECK_GL_CALL(glClearColor(0.f, 0.4f, 0.7f, 1.f));
        billboardShader->render(lightPos);
        diffuseShader->render(cube, volumeShader->getVoxelData(), lightPos);
        if (volumeShader->isEnabled()) {
			
            volumeShader->renderMesh(quad, volumePos, volumeScale, false);
        }
        if (Window::isImGuiEnabled()) {
            ImGui::Render();
        }
    }
}

void createImGuiPanes() {
    imGuiFuncs.push_back(
        [&]() {
            ImGui::Begin("Stats");
            ImGui::Text("FPS:       %d", Window::FPS);
            ImGui::Text("dt:        %f", Window::timeStep);
            glm::vec3 pos = Camera::getPosition();
            glm::vec3 look = Camera::getLookAt();
            ImGui::Text("CamPos:    (%f, %f, %f)", pos.x, pos.y, pos.z);
            ImGui::Text("CamLookAt: (%f, %f, %f)", look.x, look.y, look.z);
            ImGui::End();
        } 
    );
    imGuiFuncs.push_back(
        [&]() {
            ImGui::Begin("Light");
            ImGui::SliderFloat3("Position", glm::value_ptr(lightPos), -1000.f, 1000.f);
            ImGui::End();
        } 
    );
    imGuiFuncs.push_back(
        [&]() {
            ImGui::Begin("Billboards");
            static glm::vec3 pos(0.f);
            static float scale(0.f);
            static float rotation(0.f);
            ImGui::SliderFloat3("Position", glm::value_ptr(pos), -100.f, 100.f);
            ImGui::SliderFloat("Scale", &scale, 0.f, 100.f);
            ImGui::SliderAngle("Rotation", &rotation);
            if (ImGui::Button("Add Single Billboard")) {
                billboardShader->addCloud(pos, scale, rotation);
            }
            static int numClouds = 1;
            static float posJitter = 0.f;
            static float scaleJitter = 0.f;
            static float rotJitter = 0.f;
            ImGui::SliderInt("In Cluster", &numClouds, 1, 50);
            ImGui::SliderFloat("Position Jitter", &posJitter, 0.f, 10.f);
            ImGui::SliderFloat("Scale Jitter", &scaleJitter, 0.f, 10.f);
            ImGui::SliderFloat("Rotation Jitter", &rotJitter, 0.f, 360.f);
            if (ImGui::Button("Add Cluster")) {
                for (int i = 0; i < numClouds; i++) {
                    billboardShader->addCloud(
                        pos + Util::genRandomVec3()*posJitter,
                        scale + Util::genRandom()*scaleJitter,
                        rotation + Util::genRandom()*rotJitter);
                }
            }
            if (ImGui::Button("Clear Billboards")) {
                billboardShader->clearClouds();
            }
            bool b = billboardShader->isEnabled();
            ImGui::Checkbox("Render", &b);
            billboardShader->setEnabled(b);
            ImGui::End();
        } 
    );
    imGuiFuncs.push_back(
        [&]() {
            ImGui::Begin("Volume");
            ImGui::SliderFloat3("Position", glm::value_ptr(volumePos), -100.f, 100.f);
            //ImGui::SliderFloat3("Scale", glm::value_ptr(volumeScale), 0.f, 50.f);
            glm::vec2 x = volumeShader->getXBounds();
            glm::vec2 y = volumeShader->getYBounds();
            glm::vec2 z = volumeShader->getZBounds();
            int size = volumeShader->getVolumeSize();
            if(ImGui::SliderFloat2("XBounds", glm::value_ptr(x), -20.f, 20.f)) {
                volumeShader->setXBounds(x);
            }
            if(ImGui::SliderFloat2("YBounds", glm::value_ptr(y), -20.f, 20.f)) {
                volumeShader->setYBounds(y);
            }
            if(ImGui::SliderFloat2("ZBounds", glm::value_ptr(z), -20.f, 20.f)) {
                volumeShader->setZBounds(z);
            }
            if (ImGui::SliderInt("Volume Size", &size, 0, 256)) {
                volumeShader->setVolumeSize(size);
            }

            bool b = volumeShader->isEnabled();
            ImGui::Checkbox("Render underlying quad", &b);
            volumeShader->setEnabled(b);
            if (ImGui::Button("Quad Voxelize!")) {
                volumeShader->voxelize(quad, volumePos, volumeScale);
            }
            if (ImGui::Button("Clear Volume")) {
                volumeShader->clearVolume();
            }
            ImGui::End();
        }
    );
}

void initGeom() {
    /* Create quad */
    quad = new Mesh;
    quad->vertBuf = {
        -0.5f, -0.5f,  0.f,
         0.5f, -0.5f,  0.f,
        -0.5f,  0.5f,  0.f,
         0.5f,  0.5f,  0.f
    };
    quad->init();

    /* Create cube */
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

