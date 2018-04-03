#include "IO/Window.hpp"
#include "Camera.hpp"
#include "Util.hpp"

#include "Shaders/GLSL.hpp"
#include "Shaders/BillboardShader.hpp"
#include "Shaders/VolumeShader.hpp"
#include "Shaders/DiffuseShader.hpp"
#include "Shaders/LightDepthShader.hpp"

#include "ThirdParty/imgui/imgui.h"

#include <functional>
#include <time.h>

/* Initial values */
std::string RESOURCE_DIR = "../res/";
glm::vec3 lightPos(100.f, 100.f, -100.f);

/* Shaders */
BillboardShader * billboardShader;
VolumeShader * volumeShader;
DiffuseShader * diffuseShader;
LightDepthShader * lightDepthShader;

/* Volume quad */
Spatial volQuad(glm::vec3(5.f, 0.f, 0.f), glm::vec3(4.f), glm::vec3(0.f));

/* Geometry */
Mesh *quad;
Mesh *cube;

/* Render targets */
std::vector<Spatial *> cloudsBillboards;

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

    /* Create shaders */
    billboardShader = new BillboardShader(RESOURCE_DIR + "cloud_vert.glsl", RESOURCE_DIR + "cloud_frag.glsl");
    billboardShader->init(RESOURCE_DIR + "cloud.png", RESOURCE_DIR + "cloudMap.png", quad);
    volumeShader = new VolumeShader(RESOURCE_DIR + "voxelize_vert.glsl", RESOURCE_DIR + "voxelize_frag.glsl");
    volumeShader->init(32, glm::vec2(-8.f, 8.f), glm::vec2(-8.f, 8.f), glm::vec2(-8.f, 8.f), &volQuad);
    diffuseShader = new DiffuseShader(RESOURCE_DIR + "diffuse_vert.glsl", RESOURCE_DIR + "diffuse_frag.glsl");
    diffuseShader->init();
    lightDepthShader = new LightDepthShader(RESOURCE_DIR + "light_vert.glsl", RESOURCE_DIR + "light_frag.glsl");
    lightDepthShader->init();

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
    
        ///////////////////////////////////////////////////
        //                   RENDER                      //       
        ///////////////////////////////////////////////////
        CHECK_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        CHECK_GL_CALL(glClearColor(0.2f, 0.3f, 0.5f, 1.f));
        if (volumeShader->activeVoxelize) {
            volumeShader->voxelize(quad);
        }
        if (volumeShader->isEnabled()) {
            volumeShader->bind();
            volumeShader->renderMesh(quad, false);
            volumeShader->unbind();
        }
        billboardShader->render(lightPos, cloudsBillboards);
        diffuseShader->render(cube, volumeShader->getVoxelData(), lightPos);
        lightDepthShader->render(cube, volumeShader->getVoxelData());

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
            ImGui::SliderFloat3("Position", glm::value_ptr(lightPos), -100.f, 100.f);
            int size = lightDepthShader->lightMap->width;
            ImGui::SliderInt("Map size", &size, 512, 4096);
            lightDepthShader->setTextureSize(size);
            ImGui::Image((ImTextureID) lightDepthShader->lightMap->textureId, ImVec2(256, 256));
            ImGui::End();
        } 
    );
    imGuiFuncs.push_back(
        [&]() {
            ImGui::Begin("Billboards");
            static glm::vec3 pos(0.f);
            static float scale(1.f);
            static float rotation(0.f);
            ImGui::SliderFloat3("Position", glm::value_ptr(pos), -100.f, 100.f);
            ImGui::SliderFloat("Scale", &scale, 1.f, 10.f);
            ImGui::SliderAngle("Rotation", &rotation);
            if (ImGui::Button("Add Single Billboard")) {
                cloudsBillboards.push_back(new Spatial(
                    pos,
                    glm::vec3(scale),
                    glm::vec3(rotation)
                ));
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
                    cloudsBillboards.push_back(new Spatial(
                        pos + Util::genRandomVec3()*posJitter,
                        glm::vec3(scale + Util::genRandom()*scaleJitter),
                        glm::vec3(rotation + Util::genRandom()*rotJitter)
                    ));
               }
            }
            if (ImGui::Button("Clear Billboards")) {
                for (auto r : cloudsBillboards) {
                    delete r;
                }
                cloudsBillboards.clear();
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
            ImGui::Text("Voxels in scene : %d", volumeShader->getVoxelData().size());
            ImGui::SliderFloat3("Position", glm::value_ptr(volQuad.position), -20.f, 20.f);
            ImGui::SliderFloat("Scale", &volQuad.scale.x, 0.f, 10.f);
            ImGui::SliderFloat2("XBounds", glm::value_ptr(volumeShader->xBounds), -20.f, 20.f);
            ImGui::SliderFloat2("YBounds", glm::value_ptr(volumeShader->yBounds), -20.f, 20.f);
            ImGui::SliderFloat2("ZBounds", glm::value_ptr(volumeShader->zBounds), -20.f, 20.f);
            //ImGui::SliderInt("Volume Size", &volumeShader->volumeSize, 0, 256);

            bool b = volumeShader->isEnabled();
            ImGui::Checkbox("Render underlying quad", &b);
            volumeShader->setEnabled(b);

            ImGui::Checkbox("Voxelize!", &volumeShader->activeVoxelize);

            if (ImGui::Button("Single voxelize")) {
                volumeShader->voxelize(quad);
            }
            if (ImGui::Button("Clear")) {
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
        -1, -1, -1,
        1, 1, -1,
        1, -1, -1,
        -1, 1, -1,
        -1, -1, -1,
        -1, 1, 1,
        -1, 1, -1,
        -1, -1, 1,
        -1, 1, -1,
        1, 1, 1,
        1, 1, -1,
        -1, 1, 1,
        1, -1, -1,
        1, 1, -1,
        1, 1, 1,
        1, -1, 1,
        -1, -1, -1,
        1, -1, -1,
        1, -1, 1,
        -1, -1, 1,
        -1, -1, 1,
        1, -1, 1,
        1, 1, 1,
        -1, 1, 1
    };
    cube->norBuf = {
        0, 0, -1,
        0, 0, -1,
        0, 0, -1,
        0, 0, -1,
        -1, 0, 0,
        -1, 0, 0,
        -1, 0, 0,
        -1, 0, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        0, 1, 0,
        1, 0, 0,
        1, 0, 0,
        1, 0, 0,
        1, 0, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, -1, 0,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
        0, 0, 1,
    };
    cube->eleBuf = {
        0, 1, 2,
        0, 3, 1,
        4, 5, 6,
        4, 7, 5,
        8, 9, 10,
        8, 11, 9,
        12, 13, 14,
        12, 14, 15,
        16, 17, 18,
        16, 18, 19,
        20, 21, 22,
        20, 22, 23,
    };
    cube->init();
}

