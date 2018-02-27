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

/* Initial values */
std::string RESOURCE_DIR = "../res/";

/* Light position */
glm::vec3 lightPos;

/* Shaders */
BillboardShader *billboardShader;
VolumeShader *volumeShader;
DiffuseShader *diffuseShader;

/* Geometry */
Mesh *quad;
Mesh *cube;

/* ImGui functions */
struct ImGuiFunc { 
    std::string name;
    std::function<void()> fun;
};
std::vector<ImGuiFunc> imGuiFuncs;

void initGeom();
void createImGuiPanes();
int main() {
    /* Init window, keyboard, and mouse wrappers */
    if (Window::init("Clouds")) {
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
    volumeShader->init(32, glm::vec2(-20.f, 20.f), glm::vec2(-20.f, 15.f), glm::vec2(-12.f, 12.f));
    diffuseShader = new DiffuseShader(RESOURCE_DIR + "diffuse_vert.glsl", RESOURCE_DIR + "diffuse_frag.glsl");
    diffuseShader->init();

    /* Init ImGui Panes */
    createImGuiPanes();

    /* Init rendering */
    GLSL::checkVersion();
    CHECK_GL_CALL(glEnable(GL_DEPTH_TEST));
    CHECK_GL_CALL(glEnable(GL_BLEND));
    CHECK_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

    /* First render pass - generate volume */
    Camera::update(Window::timeStep);
    volumeShader->voxelize(billboardShader->quad, glm::vec3(5.f, 0.f, 0.f), glm::vec3(2.f));

    while (!Window::shouldClose()) {
        /* Update context */
        Window::update();

        /* Update camera */
        Camera::update(Window::timeStep);

        /* IMGUI */
        if (Window::isImGuiEnabled()) {
            for (auto func : imGuiFuncs) {
                ImGui::Begin(func.name.c_str());
                func.fun();
                ImGui::End();
            }
        }

        /* Render */
        CHECK_GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        CHECK_GL_CALL(glClearColor(0.f, 0.4f, 0.7f, 1.f));
        billboardShader->render(lightPos);
        diffuseShader->render(cube, volumeShader->getVoxelData(), lightPos);
        volumeShader->renderMesh(quad, glm::vec3(5.f, 0.f, 0.f), glm::vec3(2.f), false);
        if (Window::isImGuiEnabled()) {
            ImGui::Render();
        }
    }
}

void createImGuiPanes() {
    imGuiFuncs.push_back({ "Light",
        [&]() {
            ImGui::SliderFloat3("Position", glm::value_ptr(lightPos), -1000.f, 1000.f);
        } 
    });
    imGuiFuncs.push_back({ "Billboards",
        [&]() {
            // TODO : generate cluster w offset
            static glm::vec3 pos(0.f);
            static float scale(0.f);
            static float rotation(0.f);
            ImGui::SliderFloat3("Position", glm::value_ptr(pos), -100.f, 100.f);
            ImGui::SliderFloat("Scale", &scale, 0.f, 100.f);
            ImGui::SliderAngle("Rotation", &rotation);
            if (ImGui::Button("Add Billboard")) {
                billboardShader->addCloud(pos, scale, rotation);
            }
            if (ImGui::Button("Clear Billboards")) {
                billboardShader->clearClouds();
            }
 
        } 
    });
}

void initGeom() {
    /* Create quad */
    quad = new Mesh;
    quad->vertBuf = {
        -1.f, -1.f,  0.f,
         1.f, -1.f,  0.f,
        -1.f,  1.f,  0.f,
         1.f,  1.f,  0.f
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


