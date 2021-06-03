#ifndef __VISIBILITYCOMPUTER_H__
#define __VISIBILITYCOMPUTER_H__

#include "glm/glm.hpp"
#include <string>

class VisibilityComputer
{
private:
    static void sampleRaysThroughCell(int x, int y, glm::vec2 gridSize);
    static bool getIntersectionParameters(glm::vec2 origin, glm::vec2 dir, glm::vec2 A, glm::vec2 B, float &lambda);
public:
    static void computeAndSaveVisibility(std::string tilemapPath);
};


#endif // __VISIBILITYCOMPUTER_H__