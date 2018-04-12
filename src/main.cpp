#include "IO/Window.hpp"
#include "Camera.hpp"
#include "Util.hpp"
#include "Library.hpp"

#include "Light.hpp"

#include "Shaders/GLSL.hpp"
#include "Shaders/BillboardShader.hpp"
#include "Shaders/VoxelizeShader.hpp"
#include "Shaders/VoxelShader.hpp"

#include "ThirdParty/imgui/imgui.h"

#include <functional>
#include <time.h>

/* Initial values */
#define IMGUI_FONT_SIZE 20.f
const std::string RESOURCE_DIR = "../res/";
bool lightVoxelize = true;
bool coneTrace = true;
bool showLightView = false;
int Window::width = 1920;
int Window::height = 1080;

Spatial Light::spatial = Spatial(glm::vec3(10.f, 10.f, -10.f), glm::vec3(3.f), glm::vec3(0.f));
glm::mat4 Light::P, Light::V;
float Light::boxBounds = 10.f;
glm::vec2 Light::zBounds(0.01f, 1000.f);

Mesh * Library::cube;
Mesh * Library::quad;
Texture * Library::cloudDiffuseTexture;
Texture * Library::cloudNormalTexture;

/* Shaders */
BillboardShader * billboardShader;
VoxelizeShader * voxelizeShader;
VoxelShader * voxelShader;

/* Volume */
#define I_VOLUME_DIMENSION 32
#define I_VOLUME_BOUNDS glm::vec2(-10.f, 15.f)
#define I_VOLUME_POSITION glm::vec3(5.f, 0.f, 0.f)
#define I_VOLUME_SCALE glm::vec2(8.f)
Volume * volume;

/* Render targets */
std::vector<Spatial *> cloudsBillboards;

/* ImGui functions */
std::vector<std::function<void()>> imGuiFuncs;
void createImGuiPanes();

void exitError(std::string st) {
    std::cerr << st << std::endl;
    std::cerr << "Enter to exit: " << std::endl;
    std::cin.get();
    exit(EXIT_FAILURE);
}
int main() {
    srand((unsigned int)(time(0)));  

    /* Init window, keyboard, and mouse wrappers */
    if (Window::init("Clouds", IMGUI_FONT_SIZE)) {
        exitError("Error initializing window");
    }

    /* Create volume */
    volume = new Volume(I_VOLUME_DIMENSION, I_VOLUME_BOUNDS, I_VOLUME_POSITION, I_VOLUME_SCALE);

    /* Create meshes and textures */
    Library::init(RESOURCE_DIR + "cloud.png", RESOURCE_DIR + "cloudMap.png");

    /* Create shaders */
    billboardShader = new BillboardShader(RESOURCE_DIR + "cloud_vert.glsl", RESOURCE_DIR + "cloud_frag.glsl");
    if (!billboardShader->init()) {
        exitError("Error initializing billboard shader");
    }
    voxelizeShader = new VoxelizeShader(RESOURCE_DIR + "voxelize_vert.glsl", RESOURCE_DIR + "voxelize_frag.glsl");
    if (!voxelizeShader->init(volume, Window::width, Window::height)) {
        exitError("Error initializing volume shader");
    }
    voxelShader = new VoxelShader(RESOURCE_DIR + "voxel_vert.glsl", RESOURCE_DIR + "voxel_frag.glsl");
    if (!voxelShader->init()) {
        exitError("Error initializing voxel shader");
    }
    voxelShader->setEnabled(false);

    /* Init ImGui Panes */
    createImGuiPanes();

    /* Init rendering state */
    GLSL::checkVersion();
    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glEnable(GL_BLEND));
    CHECK_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    Camera::update(Window::timeStep);

    // add light spatial to cloud billboards so we can visualize light pos 
    // TODO : replace with proper sun rendering
    cloudsBillboards.push_back(&Light::spatial);

    while (!Window::shouldClose()) {
        /* Update context */
        Window::update();

        /* Update camera */
        Camera::update(Window::timeStep);

        /* Update light */
        Light::update(volume->quadPosition);

        /* IMGUI */
        if (Window::isImGuiEnabled()) {
            for (auto func : imGuiFuncs) {
                func();
            }
        }
    
        /* Cloud render! */
        CHECK_GL_CALL(glClearColor(0.2f, 0.3f, 0.5f, 1.f));
        CHECK_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        /* Voxelize from the light's perspective */
        if (lightVoxelize) {
            voxelizeShader->voxelize();
        }
        /* Cone trace from the camera's perspective */
        if (coneTrace) {
            voxelizeShader->coneTrace();
        }
        
        /* Render cloud billboards */
        billboardShader->render(cloudsBillboards);

        /* Render Optional */
        glm::mat4 P = showLightView ? Light::P : Camera::getP();
        glm::mat4 V = showLightView ? Light::V : Camera::getV();
        glm::vec3 camPos = showLightView ? Light::spatial.position : Camera::getPosition();
        /* Draw voxels to the screen -- optional */
        if (voxelShader->isEnabled()) {
            volume->updateVoxelData();
            voxelShader->bind();
            voxelShader->render(volume->getVoxelData(), P, V);
            voxelShader->unbind();
        }
        /* Render underlying quad -- optional*/
        if (voxelizeShader->isEnabled()) {
            voxelizeShader->bind();
            voxelizeShader->renderQuad(P, V, camPos, VoxelizeShader::None);
            voxelizeShader->unbind();
        }

        /* Render ImGui */
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
            ImGui::Checkbox("Light view", &showLightView);
            ImGui::SliderFloat3("Position", glm::value_ptr(Light::spatial.position), -100.f, 100.f);
            ImGui::SliderFloat("Bounds", &Light::boxBounds, 1.f, 100.f);
            ImGui::SliderFloat2("Near/Far", glm::value_ptr(Light::zBounds), 0.1f, 1000.f);
            static bool showLightMap = false;
            ImGui::Checkbox("Show map", &showLightMap);
            if (showLightMap) {
                ImGui::Image((ImTextureID)voxelizeShader->positionMap->textureId, ImVec2(128, 128));
            }
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
            ImGui::Begin("VXGI");
            ImGui::Text("Voxels in scene : %d", volume->voxelCount);
            ImGui::SliderFloat3("Position", glm::value_ptr(volume->quadPosition), -20.f, 20.f);
            ImGui::SliderFloat("Scale", &volume->quadScale.x, 0.f, 20.f);
            ImGui::SliderFloat2("XBounds", glm::value_ptr(volume->xBounds), -20.f, 20.f);
            ImGui::SliderFloat2("YBounds", glm::value_ptr(volume->yBounds), -20.f, 20.f);
            ImGui::SliderFloat2("ZBounds", glm::value_ptr(volume->zBounds), -20.f, 20.f);
            ImGui::SliderFloat("Step", &voxelizeShader->steps, 0.1f, 1.f);

            bool b = voxelizeShader->isEnabled();
            ImGui::Checkbox("Render underlying quad", &b);
            voxelizeShader->setEnabled(b);
            b = voxelShader->isEnabled();
            ImGui::Checkbox("Render voxels", &b);
            voxelShader->setEnabled(b);
            ImGui::Checkbox("Outlines", &voxelShader->drawOutline);

            ImGui::Checkbox("Light Voxelize!", &lightVoxelize);

            if (ImGui::Button("Single voxelize")) {
                volume->clear();
                voxelizeShader->voxelize();
            }
            if (ImGui::Button("Clear")) {
                volume->clear();
                voxelizeShader->clearPositionMap();
            }
            ImGui::SliderInt("Steps", &voxelizeShader->vctSteps, 5, 25);
            ImGui::SliderFloat("Bias", &voxelizeShader->vctBias, 0.1f, 5.f);
            ImGui::SliderFloat("Angle", &voxelizeShader->vctConeAngle, 0.f, 3.f);
            ImGui::SliderFloat("Height", &voxelizeShader->vctConeInitialHeight, 0.f, 3.f);
            ImGui::SliderFloat("LOD Offset", &voxelizeShader->vctLodOffset, 0.1f, 5.f);
            ImGui::Checkbox("Cone trace!", &coneTrace);
            ImGui::End();
        }
    );
}

