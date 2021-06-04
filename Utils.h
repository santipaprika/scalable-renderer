#include <fstream>

#include "glm/glm.hpp"

class Utils {
   public:
    static float max3(glm::vec3 v);
    static glm::vec2 getGridSize(std::ifstream &tilemap);
    static void parseGrid(std::ifstream &tilemap, int **&grid, glm::vec2 gridSize);

    template <typename T>
    static void initializePointerMatrix(T **&pointerArray, int sizeX, int sizeY)
    {
        pointerArray = new T *[sizeY];

        for (int i = 0; i < sizeY; i++)
            pointerArray[i] = new T[sizeX];
    }

    static void deletePointerMatrix(int **&pointerArray, int sizeX, int sizeY);
};
