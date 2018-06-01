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
bool showVoxels = false;
int Window::width = 1280;
int Window::height = 720;

/* Volume */
#define I_VOLUME_BOARDS 30
#define I_VOLUME_POSITION glm::vec3(5.f, 0.f, 0.f)
#define I_VOLUME_SCALE glm::vec2(10.f)
#define I_VOLUME_BOUNDS glm::vec2(-100.f, 100.f)
#define I_VOLUME_DIMENSION 32
#define I_VOLUME_MIPS 4
Volume *volume;

/* Light */
Spatial Light::spatial = Spatial(glm::vec3(250.f, 250.f, -10.f), glm::vec3(15.f), glm::vec3(0.f));
glm::mat4 Light::V(1.f);

/* Library things */
const std::string RESOURCE_DIR("../res/");
const std::string diffuseTexName("cloud.png");
const std::string normalTexName("cloudmap.png");
Mesh * Library::cube;
Mesh * Library::quad;
std::map<std::string, Texture *> Library::textures;

/* Shaders */
BillboardShader * billboardShader;
VoxelizeShader * voxelizeShader;
VoxelShader * voxelShader;
ConeTraceShader * coneShader;
Shader * debugShader;

/* Render targets */
std::vector<Spatial *> cloudsBillboards;

/* ImGui functions */
void runImGuiPanes();

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
            Util::genRandomVec3(-I_VOLUME_BOARDS, I_VOLUME_BOARDS),
            glm::vec3(Util::genRandom(15.f, 2*I_VOLUME_BOARDS)),
            glm::vec3(0.f)                      // rotation
        ));
    }

    /* Create meshes and textures */
    Library::init();
    Library::addTexture(RESOURCE_DIR, diffuseTexName);
    Library::addTexture(RESOURCE_DIR, normalTexName);
    Library::addTexture(RESOURCE_DIR, "a.png");
    Library::addTexture(RESOURCE_DIR, "a.jpg");
    Library::addTexture(RESOURCE_DIR, "b.png");

    /* Create shaders */
    billboardShader = new BillboardShader(RESOURCE_DIR, "billboard_vert.glsl", "cloud_frag.glsl");
    voxelShader = new VoxelShader(RESOURCE_DIR, "voxel_vert.glsl", "voxel_frag.glsl");
    voxelizeShader = new VoxelizeShader(RESOURCE_DIR, "billboard_vert.glsl", "first_voxelize.glsl", "second_voxelize.glsl");
    coneShader = new ConeTraceShader(RESOURCE_DIR, "billboard_vert.glsl", "conetrace_frag.glsl");
    debugShader = new Shader(RESOURCE_DIR, "billboard_vert.glsl", "debug_frag.glsl");

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

        /* Cloud render! */
        CHECK_GL_CALL(glClearColor(0.2f, 0.3f, 0.5f, 1.f));
        CHECK_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        /* Voxelize from the light's perspective */
        if (lightVoxelize) {
            volume->clearCPU();
            voxelizeShader->voxelize(volume);
        }
        /* Cone trace from the camera's perspective */
        if (coneTrace) {
            coneShader->coneTrace(volume, Window::timeStep);
        }

        /* Render cloud billboards */
        billboardShader->render(cloudsBillboards, Library::textures[diffuseTexName], Library::textures[normalTexName]);

        /* Render Optional */
        glm::mat4 V = lightView ? Light::V : Camera::getV();
        /* Draw voxels to the screen */
        if (showVoxels) {
            volume->updateVoxelData();
            voxelShader->bind();
            voxelShader->render(volume, Camera::getP(), V);
            voxelShader->unbind();
        }

        /* IMGUI */
        if (Window::isImGuiEnabled()) {
            runImGuiPanes();
            ImGui::Render();
        }
    }
}

void runImGuiPanes() {
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

    ImGui::Begin("Light");
    ImGui::SliderFloat3("Position", glm::value_ptr(Light::spatial.position), -1000.f, 1000.f);
    static float mapSize = 0.2f;
    static bool showFullMap = false;
    static bool showSmallMap = false;
    ImGui::Checkbox("Show full map", &showFullMap);
    ImGui::Checkbox("Show small map", &showSmallMap);
    if (showFullMap) {
        const Texture *posMap = voxelizeShader->positionMap;
        CHECK_GL_CALL(glClearColor(0.2f, 0.3f, 0.5f, 1.f));
        CHECK_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        debugShader->bind();
        CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0 + posMap->textureId));
        CHECK_GL_CALL(glBindTexture(GL_TEXTURE_2D, posMap->textureId));
        debugShader->loadInt(debugShader->getUniform("positionMap"), posMap->textureId);
        CHECK_GL_CALL(glBindVertexArray(Library::quad->vaoId));
        glm::mat4 M = glm::mat4(1.f);
        debugShader->loadMatrix(debugShader->getUniform("P"), &M);
        debugShader->loadMatrix(debugShader->getUniform("V"), &M);
        debugShader->loadMatrix(debugShader->getUniform("Vi"), &M);
        debugShader->loadMatrix(debugShader->getUniform("N"), &M);
        M *= glm::scale(glm::mat4(1.f), glm::vec3(2.f));
        debugShader->loadMatrix(debugShader->getUniform("M"), &M);
        CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
        CHECK_GL_CALL(glBindVertexArray(0));
        CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0));
        debugShader->unbind();
    }
    if (showSmallMap) {
        ImGui::End();
        ImGui::Begin("Position Map");
        ImGui::SliderFloat("Map Size", &mapSize, 0.1f, 1.f);
        ImGui::Image((ImTextureID)voxelizeShader->positionMap->textureId, ImVec2(voxelizeShader->positionMap->width*mapSize, voxelizeShader->positionMap->height*mapSize));
    }
    ImGui::End();

    ImGui::Begin("Billboards");
    ImGui::Checkbox("Sort", &coneShader->sort);
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

    ImGui::Begin("Cloud Volume");
    ImGui::SliderFloat3("Position", glm::value_ptr(volume->position), -20.f, 20.f);
    float minBounds[2] = { -100.f,  0.1f };
    float maxBounds[2] = { -0.1f, 100.f };
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

    ImGui::Begin("Voxels");
    ImGui::Text("Voxels in scene : %d", volume->voxelCount);
    ImGui::Checkbox("Light view", &lightView);
    if (ImGui::Checkbox("Render voxels", &showVoxels)) {
        voxelShader->disableBlack = false;
        voxelShader->disableWhite = false;
    }
    ImGui::Checkbox("Disable bounds", &voxelShader->disableBounds);
    ImGui::Checkbox("Disable black", &voxelShader->disableBlack);
    ImGui::Checkbox("Disable white", &voxelShader->disableWhite);
    ImGui::Checkbox("Voxel outlines", &voxelShader->useOutline);
    ImGui::SliderFloat("Voxel alpha", &voxelShader->alpha, 0.f, 1.f);
    ImGui::End();

    ImGui::Begin("VXGI");
    ImGui::SliderInt("Steps", &coneShader->vctSteps, 1, 30);
    ImGui::SliderFloat("Angle", &coneShader->vctConeAngle, 0.f, 3.f);
    ImGui::SliderFloat("Height", &coneShader->vctConeInitialHeight, -0.5f, 3.f);
    ImGui::SliderFloat("LOD Offset", &coneShader->vctLodOffset, 0.f, 5.f);
    ImGui::SliderFloat("Down Scaling", &coneShader->vctDownScaling, 1.f, 10.f);
    ImGui::Checkbox("Cone trace!", &coneTrace);
    ImGui::End();
}

