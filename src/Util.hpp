#pragma once
#ifndef _UTIL_HPP_
#define _UTIL_HPP_

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <algorithm>

class Util {
    public:
        static constexpr float PI = 3.14159265359;

        //////////////////////////////////////////////
        //                TIMING/FPS                //
        //////////////////////////////////////////////
        static float timeStep;
        static int FPS;
        static void updateTiming(double uTime) {
            runTime = (float)uTime;
            timeStep = (float)runTime - lastFrameTime;
            lastFrameTime = runTime;
            nFrames++;
            if (runTime - lastFpsTime >= 1.0) {
                FPS = nFrames;
                nFrames = 0;
                lastFpsTime = runTime;
            }
        }

        //////////////////////////////////////////////
        //                  RANDOM                  //
        //////////////////////////////////////////////
        /* Generate a random float [0, 1] */
        static inline float genRandom() {
            return rand() / (float) RAND_MAX;
        }
        /* Generate a scaled random value */
        static inline float genRandom(const float val) {
            return genRandom() * val;
        }
        /* Generate a random value in a range [min, max] */
        static inline float genRandom(const float min, const float max) {
            return genRandom() * (max - min) + min;
        }
        /* Generate a random vec3 with values [0, 1] */
        static inline glm::vec3 genRandomVec3() {
            return glm::vec3(genRandom(), genRandom(), genRandom());
        }
        /* Generate random vec3 with values [0, 1] */
        static inline glm::vec3 genRandomVec3(const float min, const float max) {
            return glm::vec3(genRandom(min, max), genRandom(min, max), genRandom(min, max));
        }

        //////////////////////////////////////////////
        //                 PRINTING                 //
        //////////////////////////////////////////////
        static inline void printVec3(std::string label, glm::vec3 vec) {
            std::cout << label << ": <" <<
                vec.x << ", " << vec.y << ", " << vec.z
                << ">" << std::endl;
        }


    private:
        /* Timing/FPS */
        static double lastFpsTime;
        static double lastFrameTime;
        static double runTime;
        static int nFrames;
};

#endif