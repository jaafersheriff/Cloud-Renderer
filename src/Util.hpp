#pragma once
#ifndef _UTIL_HPP_
#define _UTIL_HPP_

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <algorithm>

class Util {
    public:
        static constexpr float PI = 3.14159265359f;

        //////////////////////////////////////////////
        //                  RANDOM                  //
        //////////////////////////////////////////////
        /* Generate a random float [-1, 1] */
        static inline float genRandom() {
            return (rand() / (float) RAND_MAX - 0.5f) * 2.f;
        }
        /* Generate a random float [0, 1] */
        static inline float genPosRandom() {
            return rand() / (float) RAND_MAX;
        }
        /* Generate a scaled random value */
        static inline float genRandom(const float val) {
            return genRandom() * val;
        }
        /* Generate a random value in a range [min, max] */
        static inline float genRandom(const float min, const float max) {
            return genPosRandom() * (max - min) + min;
        }
        /* Generate a random vec3 with values [-1, 1] */
        static inline glm::vec3 genRandomVec3() {
            return glm::vec3(genRandom(), genRandom(), genRandom());
        }
        /* Generate random vec3 with values [min, max] */
        static inline glm::vec3 genRandomVec3(const float min, const float max) {
            return glm::vec3(genRandom(min, max), genRandom(min, max), genRandom(min, max));
        }
        /* Generate random vec3 with component values [min, max] */
        static inline glm::vec3 genRandomVec3(const float xmin, const float xmax, const float ymin, const float ymax, const float zmin, const float zmax) {
            return glm::vec3(genRandom(xmin, xmax), genRandom(ymin, ymax), genRandom(zmin, zmax));
        }

        //////////////////////////////////////////////
        //                 PRINTING                 //
        //////////////////////////////////////////////
        // static inline void printVec3(std::string label, glm::vec3 vec) {
        //     std::cout << label << ": <" <<
        //         vec.x << ", " << vec.y << ", " << vec.z
        //         << ">" << std::endl;
        // }
};

#endif