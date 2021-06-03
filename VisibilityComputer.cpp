#include "VisibilityComputer.h"

#include <vector>

#include "Utils.h"

#define OUT
#define N_RAYS 1000000
#define EPS 0.000001

void VisibilityComputer::computeAndSaveVisibility(std::string tilemapPath) {
    std::ifstream tilemapFile(tilemapPath);
    glm::vec2 surroundingDirs[4] = {glm::vec2(-1, 0), glm::vec2(0, -1), glm::vec2(1, 0), glm::vec2(0, 1)};

    // initialize grid
    glm::vec2 gridSize = Utils::getGridSize(tilemapFile);
    int **tilemap;
    Utils::initializePointerMatrix(tilemap, gridSize.x, gridSize.y);

    // fill grid
    Utils::parseGrid(tilemapFile, OUT tilemap, gridSize);
    tilemapFile.close();

    // explore visibility
    for (int i = 0; i < gridSize.y; i++) {
        for (int j = 0; j < gridSize.x; j++) {
            sampleRaysThroughCell(i, j, gridSize);
        }
    }
}

void VisibilityComputer::sampleRaysThroughCell(int x, int y, glm::vec2 gridSize) {
    glm::vec2 origin(x + 0.5f, y + 0.5f);

    glm::vec2 originsBB[4] = {glm::vec2(0, 0), glm::vec2(0, gridSize.y), glm::vec2(gridSize.x, gridSize.y), glm::vec2(gridSize.x, 0)};
    glm::vec2 endpointsBB[4] = {originsBB[1], originsBB[2], originsBB[3], originsBB[0]};

    for (int i = 0; i < N_RAYS; i++) {
        float angle = rand() / ((float)RAND_MAX) * 2 * M_PI;
        glm::vec2 dir = glm::vec2(cos(angle), sin(angle));
        glm::vec2 delta = abs(1.f / dir);

        int firstSide = -1;
        float lambda = INFINITY;
        for (int side = 0; side < 4; side++) {
            if (getIntersectionParameters(origin, dir, originsBB[side], endpointsBB[side], OUT lambda)) {
                if (lambda < EPS) {
                    firstSide = side;
                    break;
                }
            }
        }

        glm::vec2 startingPoint = origin + lambda * dir;
        glm::vec2 currentCell((int)startingPoint.x, (int)startingPoint.y);
        bool right = dir.x > 0;
        bool up = dir.y > 0;
        float deltaRemX = delta.x;
        float deltaRemY = delta.y;

        if (firstSide == 0 || firstSide == 2) {
            if (up)
                deltaRemY = (ceil(startingPoint.y) - startingPoint.y) * delta.y;
            else
                deltaRemY = (startingPoint.y - floor(startingPoint.y)) * delta.y;
        } else if (firstSide == 1 || firstSide == 3) {
            if (right)
                deltaRemX = (ceil(startingPoint.x) - startingPoint.x) * delta.x;
            else
                deltaRemX = (startingPoint.x - floor(startingPoint.x)) * delta.x;
        }

        std::vector<glm::vec2> visiblePath;
        while (currentCell.x < gridSize.x && currentCell.x >= 0 && currentCell.y < gridSize.y && currentCell.y >= 0) {
            visiblePath.push_back(currentCell);

            if (deltaRemX < deltaRemY) {
                right ? currentCell.x++ : currentCell.x++;
                deltaRemY -= deltaRemX;
                deltaRemX = delta.x;
            } else {
                up ? currentCell.y++ : currentCell.y--;
                deltaRemX -= deltaRemY;
                deltaRemY = delta.y;
            }
        }
    }
}

bool VisibilityComputer::getIntersectionParameters(glm::vec2 origin, glm::vec2 dir, glm::vec2 A, glm::vec2 B, float &lambda) {
    glm::vec2 sideDir = B - A;

    float dirCross = dir.x * sideDir.y - dir.y * sideDir.x;
    if (dirCross < EPS)
        return false;

    glm::vec2 AminO = A - origin;
    lambda = (AminO.x * sideDir.y - AminO.y * sideDir.x) / dirCross;  // segment parameter
    float u = (AminO.x * dir.y - AminO.y * dir.x) / dirCross;           // ray lambda

    if (u < 0 || u > 1)
        return false;

    return true;
}
