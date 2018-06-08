/* Parent Camera class
 * This camera can be used as a floating camera with no bounds */
#pragma once
#ifndef _CAMERA_HPP_
#define _CAMERA_HPP_

#include "glm/glm.hpp"

#define LOOK_SPEED 0.004f
#define MOVE_SPEED 0.25f

class Camera {
    public:
        /* Update */
        static void update();

        /* Move according to UVW */
        static void moveForward(const float);
        static void moveBackward(const float);
        static void moveLeft(const float);
        static void moveRight(const float);
        static void moveUp(const float);
        static void moveDown(const float);

        static glm::mat4 & getP() { return P; }
        static glm::mat4 & getV() { return V; }

        static glm::vec3 getPosition() { return position; }
        static glm::vec3 getLookAt() { return lookAt; }

    protected:
        /* Used for look at calculation */
        static double phi;
        static double theta;

        /* UVW basis vectors */
        static glm::vec3 u, v, w;

        /* Matrices */
        static glm::mat4 P;
        static glm::mat4 V;

        /* Positions */
        static glm::vec3 position;
        static glm::vec3 lookAt;
};

#endif
