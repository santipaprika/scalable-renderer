#ifndef __COLOR_H__
#define __COLOR_H_

#include "glm/glm.hpp"

class Color {
   public:
    static glm::vec3 red() { return glm::vec3(1.f, 0.f, 0.f); }
    static glm::vec3 redyellow() { return glm::vec3(1.f, 0.5f, 0.f); }
    static glm::vec3 yellow() { return glm::vec3(1.f, 1.f, 0.f); }
    static glm::vec3 yellowgreen() { return glm::vec3(0.5f, 1.f, 0.f); }
    static glm::vec3 green() { return glm::vec3(0.f, 1.f, 0.f); }
};

#endif  // __COLOR_H__