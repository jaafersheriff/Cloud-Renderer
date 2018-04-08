#include "IO/Window.hpp"
#include "Camera.hpp"
#include "Util.hpp"
#include "Library.hpp"

#include "Light.hpp"

#include "Shaders/GLSL.hpp"
#include "Shaders/BillboardShader.hpp"
#include "Shaders/VolumeShader.hpp"
#include "Shaders/VoxelShader.hpp"
#include "Shaders/LightMapWriteShader.hpp"

#include "ThirdParty/imgui/imgui.h"

#include <functional>
#include <time.h>

/* Initial values */
#define IMGUI_FONT_SIZE 13.f
#define I_VOLUME_BOUNDS glm::vec2(-10.f, 10.f)
#define I_VOLUME_VOXELS 32
const std::string RESOURCE_DIR = "../res/";
bool lightVoxelize = false;

Spatial Light::spatial = Spatial(glm::vec3(10.f, 10.f, -10.f), glm::vec3(10.f), glm::vec3(0.f));
glm::mat4 Light::P(1.f);
glm::mat4 Light::V(1.f);
float Light::boxBounds = 10.f;
glm::vec2 Light::zBounds(0.01f, 1000.f);

Mesh * Library::cube;
Mesh * Library::quad;
Texture * Library::cloudDiffuseTexture;
Texture * Library::cloudNormalTexture;

/* Shaders */
BillboardShader * billboardShader;
VolumeShader * volumeShader;
VoxelShader * voxelShader;
LightMapWriteShader * lightWriteShader;

/* Volume quad */
Spatial volQuad(glm::vec3(5.f, 0.f, 0.f), glm::vec3(4.f), glm::vec3(0.f));

/* Render targets */
std::vector<Spatial *> cloudsBillboards;

/* ImGui functions */
std::vector<std::function<void()>> imGuiFuncs;

void createImGuiPanes();
int main() {
    srand((unsigned int)(time(0)));  

    /* Init window, keyboard, and mouse wrappers */
    if (Window::init("Clouds", IMGUI_FONT_SIZE)) {
        std::cerr << "ERROR" << std::endl;
        std::cin.get();
        return 1;
    }

    /* Create meshes and textures */
    Library::init(RESOURCE_DIR + "cloud.png", RESOURCE_DIR + "cloudMap.png");

    /* Create shaders */
    billboardShader = new BillboardShader(RESOURCE_DIR + "cloud_vert.glsl", RESOURCE_DIR + "cloud_frag.glsl");
    billboardShader->init();
    volumeShader = new VolumeShader(RESOURCE_DIR + "voxelize_vert.glsl", RESOURCE_DIR + "voxelize_frag.glsl");
    volumeShader->init(I_VOLUME_VOXELS, I_VOLUME_BOUNDS, I_VOLUME_BOUNDS, I_VOLUME_BOUNDS, &volQuad);
    voxelShader = new VoxelShader(RESOURCE_DIR + "voxel_vert.glsl", RESOURCE_DIR + "voxel_frag.glsl");
    voxelShader->init();
    lightWriteShader = new LightMapWriteShader(RESOURCE_DIR + "light_vert.glsl", RESOURCE_DIR + "light_frag.glsl");
    lightWriteShader->init();

    /* Init ImGui Panes */
    createImGuiPanes();

    /* Init rendering */
    GLSL::checkVersion();
    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glEnable(GL_BLEND));
    CHECK_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    Camera::update(Window::timeStep);

    ///////////////////////////////////////////////////////////////////////////////
    ////// add light spatial to cloud billboards so we can visualize light pos ////
    ///////////////////////////////////////////////////////////////////////////////
    cloudsBillboards.push_back(&Light::spatial);

    while (!Window::shouldClose()) {
        /* Update context */
        Window::update();

        /* Update camera */
        Camera::update(Window::timeStep);

        /* Update light */
        Light::update(volQuad.position);

        /* IMGUI */
        if (Window::isImGuiEnabled()) {
            for (auto func : imGuiFuncs) {
                func();
            }
        }
    
        ///////////////////////////////////////////////////
        //                   RENDER                      //       
        ///////////////////////////////////////////////////
        CHECK_GL_CALL(glClearColor(0.2f, 0.3f, 0.5f, 1.f));
        CHECK_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        /* Voxelize from the light's perspective */
        if (lightVoxelize) {
            /* Clear volume */
            volumeShader->clearVolume();
            /* Voxelize from light source with black voxels */
            volumeShader->voxelize(Light::P, Light::V, Light::spatial.position, 0);
            /* Create position FBO */
            lightWriteShader->render(volumeShader->getVoxelData());
            /* Voxelize from light source with white voxels */
            volumeShader->voxelize(Light::P, Light::V, Light::spatial.position, lightWriteShader->lightMap->textureId);
        }

        /* Draw voxels to the screen */
        voxelShader->render(volumeShader->getVoxelData());

        /* Render underlying quad */
        if (volumeShader->isEnabled()) {
            volumeShader->bind();
            volumeShader->renderMesh(Camera::getP(), Camera::getV(), Camera::getPosition(), false, 0);
            volumeShader->unbind();
        }

        billboardShader->render(cloudsBillboards);

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
            if (ImGui::Button("Vsync")) {
                Window::toggleVsync();
            }
            ImGui::End();
        } 
    );
    imGuiFuncs.push_back(
        [&]() {
            ImGui::Begin("Light");
            ImGui::SliderFloat3("Position", glm::value_ptr(Light::spatial.position), -100.f, 100.f);
            ImGui::SliderFloat("Bounds", &Light::boxBounds, 1.f, 100.f);
            ImGui::SliderFloat2("Near/Far", glm::value_ptr(Light::zBounds), 0.1f, 1000.f);
            ImGui::Image((ImTextureID) lightWriteShader->lightMap->textureId, ImVec2(256, 256));
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
            ImGui::SliderFloat("Step", &volumeShader->normalStep, 0.05f, 0.5f);
            ImGui::SliderFloat("Contrib", &volumeShader->visibilityContrib, 0.f, 0.2f);
            //ImGui::SliderInt("Volume Size", &volumeShader->volumeSize, 0, 256);

            bool b = volumeShader->isEnabled();
            ImGui::Checkbox("Render underlying quad", &b);
            volumeShader->setEnabled(b);
            b = voxelShader->isEnabled();
            ImGui::Checkbox("Render voxels", &b);
            voxelShader->setEnabled(b);
            ImGui::Checkbox("Outlines", &voxelShader->drawOutline);

            ImGui::Checkbox("Light Voxelize!", &lightVoxelize);

            if (ImGui::Button("Single voxelize")) {
                volumeShader->clearVolume();
                volumeShader->voxelize(Light::P, Light::V, Light::spatial.position, 0);
                lightWriteShader->render(volumeShader->getVoxelData());
                volumeShader->voxelize(Light::P, Light::V, Light::spatial.position, lightWriteShader->lightMap->textureId);
            }
            if (ImGui::Button("Clear")) {
                volumeShader->clearVolume();
            }
            ImGui::End();
        }
    );
}

