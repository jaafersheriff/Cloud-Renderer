#include "IO/Window.hpp"
#include "Camera.hpp"
#include "Util.hpp"
#include "Library.hpp"

#include "Light.hpp"
#include "Volume.hpp"

#include "Shaders/GLSL.hpp"
#include "Shaders/BillboardShader.hpp"
#include "Shaders/VoxelizeShader.hpp"
#include "Shaders/VoxelShader.hpp"
#include "Shaders/ConeTraceShader.hpp"

#include "ThirdParty/imgui/imgui.h"

#include <functional>
#include <time.h>

/* Initial values */
#define IMGUI_FONT_SIZE 13.f
bool lightVoxelize = true;
bool coneTrace = true;
bool lightView = false;
int Window::width = 1280;
int Window::height = 720;

/* Volume */
#define I_VOLUME_BOARDS 5
#define I_VOLUME_POSITION glm::vec3(5.f, 0.f, 0.f)
#define I_VOLUME_SCALE glm::vec2(10.f)
#define I_VOLUME_BOUNDS glm::vec2(-100.f, 100.f)
#define I_VOLUME_DIMENSION 32
#define I_VOLUME_MIPS 4
Volume *volume;

/* Light */
Spatial Light::spatial = Spatial(glm::vec3(10.f, 10.f, -10.f), glm::vec3(3.f), glm::vec3(0.f));
glm::mat4 Light::V(1.f);

/* Library things */
const std::string RESOURCE_DIR("../res/");
const std::string diffuseTexName(RESOURCE_DIR + "cloud.png");
const std::string normalTexName(RESOURCE_DIR + "cloudmap.png");
Mesh * Library::cube;
Mesh * Library::quad;
std::map<std::string, Texture *> Library::textures;

/* Shaders */
BillboardShader * billboardShader;
VoxelizeShader * voxelizeShader;
VoxelShader * voxelShader;
ConeTraceShader * coneShader;

/* Render targets */
std::vector<Spatial *> cloudsBillboards;

/* ImGui functions */
std::vector<std::function<void()>> imGuiFuncs;
void createImGuiPanes();

void exitError(std::string st) {
    std::cerr << st << std::endl;
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
    volume = new Volume(I_VOLUME_DIMENSION, I_VOLUME_BOUNDS, I_VOLUME_POSITION, I_VOLUME_MIPS);
    for (int i = 0; i < I_VOLUME_BOARDS; i++) {
        volume->addCloudBoard(Spatial(
            Util::genRandomVec3(-10.f, 15.f),   // position offset
            glm::vec3(15.f),                    // scale
            glm::vec3(0.f)                      // rotation
        ));
    }

    /* Create meshes and textures */
    Library::init();
    Library::addTexture(diffuseTexName);
    Library::addTexture(normalTexName);

    /* Create shaders */
    billboardShader = new BillboardShader(RESOURCE_DIR + "billboard_vert.glsl", RESOURCE_DIR + "cloud_frag.glsl");
    voxelShader = new VoxelShader(RESOURCE_DIR + "voxel_vert.glsl", RESOURCE_DIR + "voxel_frag.glsl");
    voxelShader->setEnabled(false);
    voxelizeShader = new VoxelizeShader(RESOURCE_DIR + "billboard_vert.glsl", RESOURCE_DIR + "first_voxelize.glsl", RESOURCE_DIR + "second_voxelize.glsl");
    coneShader = new ConeTraceShader(RESOURCE_DIR + "billboard_vert.glsl", RESOURCE_DIR + "conetrace_frag.glsl");

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
        Light::update(volume->position);

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
            if (voxelShader->isEnabled()) {
                volume->clearCPU();
            }
            voxelizeShader->voxelize(volume);
        }
        /* Cone trace from the camera's perspective */
        if (coneTrace) {
            coneShader->coneTrace(volume);
        }

        /* Render cloud billboards */
        billboardShader->render(cloudsBillboards, Library::textures[diffuseTexName], Library::textures[normalTexName]);

        /* Render Optional */
        glm::mat4 V = lightView ? Light::V : Camera::getV();
        /* Draw voxels to the screen */
        if (voxelShader->isEnabled()) {
            volume->updateVoxelData();
            voxelShader->bind();
            voxelShader->render(volume, Camera::getP(), V);
            voxelShader->unbind();
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
            ImGui::SliderFloat3("Position", glm::value_ptr(Light::spatial.position), -1000.f, 1000.f);
            static float mapSize = 0.1f;
            static bool showLightMap = true;
            ImGui::Checkbox("Show map", &showLightMap);
            if (showLightMap) {
                ImGui::End();
                ImGui::Begin("Position Map");
                ImGui::SliderFloat("Map Size", &mapSize, 0.1f, 1.f);
                ImGui::Image((ImTextureID)voxelizeShader->positionMap->textureId, ImVec2(voxelizeShader->positionMap->width*mapSize, voxelizeShader->positionMap->height*mapSize));
            }
            ImGui::End();
        } 
    );
    imGuiFuncs.push_back(
        [&]() {
            ImGui::Begin("Billboards");
            static glm::vec3 newPos(0.f);
            static float scale = 1.f;
            ImGui::SliderFloat3("Offset", glm::value_ptr(newPos), -200.f, 200.f);
            ImGui::SliderFloat("Scale", &scale, 1.f, 200.f);
            if (ImGui::Button("Add Billboard")) {
                volume->addCloudBoard(Spatial(newPos, glm::vec3(scale), glm::vec3(0.f)));
            }
            static int currBoard = 0;
            ImGui::SliderInt("Curr board", &currBoard, 0, volume->cloudBoards.size() - 1);
            Spatial *s = &volume->cloudBoards[currBoard];
            ImGui::SliderFloat3("Position", glm::value_ptr(s->position), -200.f, 200.f);
            ImGui::SliderFloat("Cscale", &s->scale.x, 1.f, 200.f);
            if (ImGui::Button("Delete") && volume->cloudBoards.size()) {
                volume->cloudBoards.erase(volume->cloudBoards.begin() + currBoard);
                currBoard = glm::max(0, currBoard - 1);
            }
            ImGui::End();
        } 
    );
    imGuiFuncs.push_back(
        [&]() {
            ImGui::Begin("Cloud Volume");
            ImGui::SliderFloat3("Position", glm::value_ptr(volume->position), -20.f, 20.f);
            float minBounds[2] = { -100.f,  0.1f };
            float maxBounds[2] = {  -0.1f, 100.f };
            ImGui::SliderFloat2("XBounds", glm::value_ptr(volume->xBounds), minBounds, maxBounds);
            ImGui::SliderFloat2("YBounds", glm::value_ptr(volume->yBounds), minBounds, maxBounds);
            ImGui::SliderFloat2("ZBounds", glm::value_ptr(volume->zBounds), minBounds, maxBounds);
            ImGui::Checkbox("Light Voxelize!", &lightVoxelize);
            if (ImGui::Button("Single voxelize")) {
                volume->clearCPU();
                voxelizeShader->voxelize(volume);
            }
            if (ImGui::Button("Clear")) {
                volume->clearGPU();
                volume->clearCPU();
                voxelizeShader->clearPositionMap();
            }
            ImGui::End();
        }
    );
    imGuiFuncs.push_back(
        [&]() {
            ImGui::Begin("Voxels");
            ImGui::Text("Voxels in scene : %d", volume->voxelCount);
            ImGui::Checkbox("Light view", &lightView);
            bool b = voxelShader->isEnabled();
            if (ImGui::Checkbox("Render voxels", &b)) {
                voxelShader->setEnabled(b);
                voxelShader->disableBlack = false;
                voxelShader->disableWhite = false;
            }
            ImGui::Checkbox("Disable bounds", &voxelShader->disableBounds);
            ImGui::Checkbox("Disable black", &voxelShader->disableBlack);
            ImGui::Checkbox("Disable white", &voxelShader->disableWhite);
            ImGui::Checkbox("Voxel outlines", &voxelShader->useOutline);
            ImGui::SliderFloat("Voxel alpha", &voxelShader->alpha, 0.f, 1.f);
            ImGui::End();
        }
    );
    imGuiFuncs.push_back(
        [&]() {
            ImGui::Begin("VXGI");
            ImGui::SliderInt("Steps", &coneShader->vctSteps, 1, 30);
            ImGui::SliderFloat("Angle", &coneShader->vctConeAngle, 0.f, 3.f);
            ImGui::SliderFloat("Height", &coneShader->vctConeInitialHeight, -0.5f, 3.f);
            ImGui::SliderFloat("LOD Offset", &coneShader->vctLodOffset, 0.f, 5.f);
            ImGui::Checkbox("Cone trace!", &coneTrace);
            ImGui::End();
        }
    );
 
}

