#include "glm/glm.hpp"
#include <fstream>


class Utils
{
public:
    static float max3(glm::vec3 v);
    static glm::vec2 getGridSize(std::ifstream &tilemap);
    static void parseGrid(std::ifstream &tilemap, int **grid, glm::vec2 gridSize);
};
