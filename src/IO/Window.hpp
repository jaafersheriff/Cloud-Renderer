/* GLFW Handler
* Maintains GLFW window, mouse, and keyboard*/
#pragma once
#ifndef _GLFW_HANDLER_HPP_
#define _GLFW_HANDLER_HPP_

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Mouse.hpp"
#include "Keyboard.hpp"

#include <string>

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 960

class Window {
public:
    /* Window size */
    static int width;
    static int height;

    /* Init */
    static int init(std::string);

    /* Set window title */
    void setTitle(const char *);

    /* Update */
    static void update();

    /* Return if window should close */
    static int shouldClose();

    /* Return running time */
    static double getTime();

    /* Shut down */
    void shutDown();

    /* Timing */
    static float timeStep;
    static int FPS;

    /* ImGui */
    static void toggleImgui() { imGuiEnabled = !imGuiEnabled; }
    static bool isImGuiEnabled() { return imGuiEnabled; }

private:
    /* Reference to GLFW window, mouse, keyboard */
    static GLFWwindow *window;

    /* Callback functions */
    static void errorCallback(int, const char *);
    static void keyCallback(GLFWwindow *, int, int, int, int);
    static void mouseButtonCallback(GLFWwindow *, int, int, int);
    static void characterCallback(GLFWwindow *, unsigned int);

    /* Timing */
    static double lastFpsTime;
    static double lastFrameTime;
    static double runTime;
    static int nFrames;

    /* ImGui */
    static bool imGuiEnabled;
    static float imGuiTimer;
};

#endif