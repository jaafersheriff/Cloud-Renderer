#pragma once
#ifndef _SUN_HPP_
#define _SUN_HPP_

#include "Spatial.hpp"

#include "glm/gtc/matrix_transform.hpp"

class Sun {
    public:
        static Spatial spatial;
        static glm::mat4 V;

        static glm::vec3 innerColor;
        static glm::vec3 outerColor;

        Sun(Spatial &spat, glm::vec3 inC, float inR, glm::vec3 outC, float outR) {
            spatial = spat;
            V = glm::mat4(1.f);
            innerColor = inC;
            outerColor = outC;
            setInnerRadius(inR);
            setOuterRadius(outR);
        }

        static void update(glm::vec3 lookAt) {
            V = glm::lookAt(spatial.position, lookAt, glm::vec3(0, 1, 0));
        }

        static void setInnerRadius(float in) {
            if (innerRadius + in >= 0 && innerRadius + in <= outerRadius) {
                innerRadius += in;
            }
        }

        static void setOuterRadius(float out) {
            if (outerRadius + out >= 0) {
                outerRadius += out;
            }
            if (outerRadius + out <= innerRadius) {
                setInnerRadius(out);
            }
            spatial.scale = glm::vec3(out);
        }

        static float getInnerRadius() { return innerRadius; }
        static float getOuterRadius() { return outerRadius; }

    protected:
        static float innerRadius;
        static float outerRadius;
};

#endif
