/* Mouse class
* Maintains mouse movement inside GLFW window */
#pragma once
#ifndef _MOUSE_HPP_
#define _MOUSE_HPP_

#include <GLFW/glfw3.h>

class Mouse {
public:
    /* x-y position and speed */
    static double x, y;
    static double dx, dy;

    static void update(double, double);

    /* Denotes if mouse buttons are pressed */
    static void setButtonStatus(int, int);
    static bool isDown(int);
    static bool isUp(int);
private:
    static int mouseButtons[GLFW_MOUSE_BUTTON_LAST];
};

#endif