#include "IO/Window.hpp"
#include "Camera.hpp"
#include "Util.hpp"
#include "Library.hpp"

#include "Sun.hpp"
#include "CloudVolume.hpp"

#include "Shaders/GLSL.hpp"
#include "Shaders/SunShader.hpp"
#include "Shaders/VoxelizeShader.hpp"
#include "Shaders/VoxelShader.hpp"
#include "Shaders/ConeTraceShader.hpp"

#include "ThirdParty/imgui/imgui.h"

#include <functional>
#include <time.h>

/* Initial values */
#define IMGUI_FONT_SIZE 13.f
bool lightVoxelize = true;
bool lightView = false;
bool showVoxels = false;
int Window::width = 1280;
int Window::height = 720;

/* Volume */
const int I_VOLUME_BOARDS = 200;
const glm::vec3 I_VOLUME_POSITION = glm::vec3(25.f, 0.f, 0.f);
const glm::vec2 I_VOLUME_BOUNDS = glm::vec2(-5.f, 5.f);
const int I_VOLUME_DIMENSION = 32;
const int I_VOLUME_MIPS = 4;
CloudVolume *volume;

/* Sun */
glm::vec3 Sun::position = glm::vec3(5.f, 20.f, -5.f);
glm::mat4 Sun::P = glm::mat4(1.f);
glm::mat4 Sun::V = glm::mat4(1.f);
glm::vec3 Sun::innerColor = glm::vec3(1.f);
glm::vec3 Sun::outerColor = glm::vec3(1.f, 1.f, 0.f);
float Sun::innerRadius = 1.f;
float Sun::outerRadius = 2.f;
glm::vec3 Sun::nearPlane;
glm::vec3 Sun::farPlane;
float Sun::clipDistance;

/* Library things */
const std::string RESOURCE_DIR("../res/");
Mesh * Library::quad;
std::map<std::string, Texture *> Library::textures;

/* Shaders */
SunShader * sunShader;
VoxelizeShader * voxelizeShader;
VoxelShader * voxelShader;
ConeTraceShader * coneShader;
Shader * debugShader;

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

    /* Init library */
    Library::init();

    /* Create volume */
    volume = new CloudVolume(I_VOLUME_DIMENSION, I_VOLUME_BOUNDS, I_VOLUME_POSITION, I_VOLUME_MIPS);
    for (int i = 0; i < I_VOLUME_BOARDS; i++) {
        volume->addCloudBoard(
            Util::genRandomVec3(-2.5f, 2.5f),
            (Util::genRandom(1.f, 2.5f)));
    }

    /* Create shaders */
    sunShader = new SunShader(RESOURCE_DIR, "billboard_vert.glsl", "sun_frag.glsl");
    voxelShader = new VoxelShader(volume->dimension, RESOURCE_DIR, "voxel_vert.glsl", "voxel_frag.glsl");
    voxelizeShader = new VoxelizeShader(RESOURCE_DIR, "billboard_vert_instanced.glsl", "billboard_vert.glsl", "first_voxelize.glsl", "second_voxelize.glsl");
    coneShader = new ConeTraceShader(RESOURCE_DIR, "billboard_vert_instanced.glsl", "conetrace_frag.glsl");
    debugShader = new Shader(RESOURCE_DIR, "billboard_vert.glsl", "debug_frag.glsl");

    /* Init rendering state */
    GLSL::checkVersion();
    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glEnable(GL_BLEND));
    CHECK_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    Camera::update();

    while (!Window::shouldClose()) {
        /* Update context */
        Window::update();

        /* Update camera */
        Camera::update();

        /* Update light */
        Sun::update(volume);

        /* Update volume */
        volume->update();

        /* Cloud render! */
        CHECK_GL_CALL(glClearColor(0.2f, 0.3f, 0.5f, 1.f));
        CHECK_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        /* Render sun */
        sunShader->render();

        /* Voxelize from the light's perspective */
        if (lightVoxelize) {
            voxelizeShader->voxelize(volume);
        }
        /* Cone trace from the camera's perspective */
        coneShader->coneTrace(volume);

        /* Render Optional */
        glm::mat4 P = lightView ? Sun::P : Camera::getP();
        glm::mat4 V = lightView ? Sun::V : Camera::getV();
        /* Draw voxels to the screen */
        if (showVoxels) {
            voxelShader->bind();
            voxelShader->render(volume, P, V);
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
    {
        ImGui::Text("FPS:       %d", Window::FPS);
        ImGui::Text("Avg FPS:   %0.4f", (float)Window::totalFrames / Window::runTime);
        ImGui::Text("dt:        %0.4f", Window::timeStep);
        glm::vec3 pos = Camera::getPosition();
        ImGui::Text("CamPos:    (%0.2f, %0.2f, %0.2f)", pos.x, pos.y, pos.z);
        if (ImGui::Button("Vsync")) {
            Window::toggleVsync();
        }
    }
    ImGui::End();

    static bool showFullMap = false;
    static bool showSmallMap = false;
    ImGui::Begin("Sun");
    {
        ImGui::SliderFloat3("Position", glm::value_ptr(Sun::position), -100.f, 100.f);
        ImGui::SliderFloat3("Inner Color", glm::value_ptr(Sun::innerColor), 0.f, 1.f);
        ImGui::SliderFloat3("Outer Color", glm::value_ptr(Sun::outerColor), 0.f, 1.f);
        ImGui::SliderFloat("Inner Radius", &Sun::innerRadius, 0.f, 10.f);
        ImGui::SliderFloat("Outer Radius", &Sun::outerRadius, 0.f, 10.f);
        ImGui::Checkbox("Show full map", &showFullMap);
        ImGui::Checkbox("Show small map", &showSmallMap);
    }
    ImGui::End();

    /* Render full-screen map */
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
        debugShader->loadMatrix(debugShader->getUniform("M"), &M);
        CHECK_GL_CALL(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
        CHECK_GL_CALL(glBindVertexArray(0));
        CHECK_GL_CALL(glActiveTexture(GL_TEXTURE0));
        debugShader->unbind();
    }
    if (showSmallMap) {
        ImGui::Begin("Position Map");
        static float mapSize = 0.2f;
        ImGui::SliderFloat("Map Size", &mapSize, 0.1f, 1.f);
        ImGui::Image((ImTextureID)voxelizeShader->positionMap->textureId, ImVec2(voxelizeShader->positionMap->width*mapSize, voxelizeShader->positionMap->height*mapSize));
        ImGui::End();
    }

    ImGui::Begin("Billboards");
    {
        ImGui::Checkbox("Show", &coneShader->showQuad);
        static glm::vec3 newPos(0.f);
        static float scale = 1.f;
        ImGui::SliderFloat3("Offset", glm::value_ptr(newPos), -10.f, 10.f);
        ImGui::SliderFloat("Scale", &scale, 0.1f, 10.f);
        if (ImGui::Button("Add Billboard")) {
            volume->addCloudBoard(newPos, scale);
        }
        static glm::vec2 ranPos = glm::vec2(-3.f, 3.f);
        static glm::vec2 ranScale = glm::vec2(1.f, 3.f);
        bool changing = false;
        static int numBoards = I_VOLUME_BOARDS;
        changing |= ImGui::SliderFloat2("Random Offset", glm::value_ptr(ranPos), -5.f, 5.f);
        changing |= ImGui::SliderFloat2("Random Scale", glm::value_ptr(ranScale), 1.f, 5.f);
        changing |= ImGui::SliderInt("Number billboards", &numBoards, 0, I_VOLUME_BOARDS);
        if (ImGui::Button("Reset billboards") || changing) {
            volume->billboardPositions.clear();
            volume->billboardScales.clear();
            for (int i = 0; i < numBoards; i++) {
                volume->addCloudBoard(
                    Util::genRandomVec3(ranPos.x, ranPos.y),
                    Util::genRandom(ranScale.x, ranScale.y)
                );
            }
        }
        if (volume->billboardPositions.size()) {
            static int currBoard = 0;
            ImGui::SliderInt("Curr board", &currBoard, 0, volume->billboardPositions.size() - 1);
            glm::vec3 *currPos = &volume->billboardPositions[currBoard];
            float *currScale = &volume->billboardScales[currBoard];
            ImGui::SliderFloat3("Position", glm::value_ptr(*currPos), -10.f, 10.f);
            ImGui::SliderFloat("Cscale", currScale, 1.f, 10.f);
            if (ImGui::Button("Delete") && volume->billboardPositions.size()) {
                volume->billboardPositions.erase(volume->billboardPositions.begin() + currBoard);
                volume->billboardScales.erase(volume->billboardScales.begin() + currBoard);
                currBoard = glm::max(0, currBoard - 1);
            }
        }
    }
    ImGui::End();

    ImGui::Begin("Volume");
    {
        ImGui::SliderFloat3("Position", glm::value_ptr(volume->position), -20.f, 20.f);
        float minBounds[2] = { -10.f,-0.1f };
        float maxBounds[2] = { 0.1f, 10.f };
        ImGui::SliderFloat2("XBounds", glm::value_ptr(volume->xBounds), minBounds, maxBounds);
        ImGui::SliderFloat2("YBounds", glm::value_ptr(volume->yBounds), minBounds, maxBounds);
        ImGui::SliderFloat2("ZBounds", glm::value_ptr(volume->zBounds), minBounds, maxBounds);
        static float scale = 10.f;
        if (ImGui::SliderFloat("Scale", &scale, 0.f, 50.f)) {
            volume->xBounds.x = -scale;
            volume->xBounds.y =  scale;
            volume->yBounds.x = -scale;
            volume->yBounds.y =  scale;
            volume->zBounds.x = -scale;
            volume->zBounds.y =  scale;
        }
    }
    ImGui::End();

    ImGui::Begin("Voxels");
    {
        ImGui::Text("Voxels in scene : %d", voxelShader->activeVoxels);
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
        ImGui::Checkbox("Light Voxelize!", &lightVoxelize);
    }
    ImGui::End();

    ImGui::Begin("VXGI");
    {
        ImGui::SliderInt("Steps", &coneShader->vctSteps, 1, 30);
        ImGui::SliderFloat("Angle", &coneShader->vctConeAngle, 0.f, 3.f);
        ImGui::SliderFloat("Height", &coneShader->vctConeInitialHeight, -0.5f, 3.f);
        ImGui::SliderFloat("LOD Offset", &coneShader->vctLodOffset, 0.f, 5.f);
        ImGui::SliderFloat("Down Scaling", &coneShader->vctDownScaling, 1.f, 10.f);
        ImGui::Checkbox("Cone trace!", &coneShader->doConeTrace);
    }
    ImGui::End();

    ImGui::Begin("Noise");
    {
        ImGui::Checkbox("Noise sample", &coneShader->doNoiseSample);
        ImGui::SliderFloat("Step size", &coneShader->stepSize, 0.001f, 10.f);
        ImGui::SliderFloat("Noise opacity", &coneShader->noiseOpacity, 0.1f, 40.f);
        ImGui::SliderInt("Octaves", &coneShader->numOctaves, 1, 10);
        ImGui::SliderFloat("Frequency", &coneShader->freqStep, 0.01f, 10.f);
        ImGui::SliderFloat("Persistence", &coneShader->persStep, 0.01f, 1.f);
        ImGui::SliderFloat3("Wind Dir", glm::value_ptr(coneShader->windVel), -0.05f, 0.05f);
    }
    ImGui::End();

}

