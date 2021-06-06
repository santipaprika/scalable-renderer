#include "VisibilityComputer.h"

#include <vector>
#include <iostream>
#include "Utils.h"
#include "Scene.h"

#define OUT
#define N_RAYS 200
#define EPS 0.000001

std::unordered_set<glm::vec2>** VisibilityComputer::visibilityPerCell; 
void VisibilityComputer::computeAndSaveVisibility(std::string tilemapPath) {
    std::ifstream tilemapFile(tilemapPath);

    // initialize grid
    glm::vec2 gridSize = Utils::getGridSize(tilemapFile);
    int **tilemap;
    Utils::initializePointerMatrix<int>(tilemap, gridSize.x, gridSize.y);

    // fill grid
    Utils::parseGrid(tilemapFile, OUT tilemap, gridSize);
    tilemapFile.close();

    Utils::initializePointerMatrix<std::unordered_set<glm::vec2>>(visibilityPerCell, gridSize.x, gridSize.y);

    // explore visibility
    float percent = 0;
    for (int i = 0; i < gridSize.x; i++) {
        if (i % (max(1,(int)gridSize.x/100)) == 0) {
            percent = (i*gridSize.y) / (gridSize.x*gridSize.y);
            std::cout << "[ " << percent*100 << " %] Generating visibility per cell" << std::endl;
        }
        for (int j = 0; j < gridSize.y; j++) {
            visibilityPerCell[j][i].insert(glm::vec2(j,i));
            sampleRaysThroughCell(i, j, gridSize, tilemap);
        }
    }

    std::cout << "Writing..." << std::endl;
    if (writeVisibilityFile(gridSize))
        std::cout << "Done!" << std::endl;
    else
        std::cout << "E1: Could not write visibility file!" << std::endl;

    Utils::deletePointerMatrix<std::unordered_set<glm::vec2>>(visibilityPerCell, gridSize.x, gridSize.y);
    Utils::deletePointerMatrix<int>(tilemap, gridSize.x, gridSize.y);
}

void VisibilityComputer::sampleRaysThroughCell(int x, int y, glm::vec2 gridSize, int** tilemap) {
    glm::vec2 originFix((float)x,(float)y);

    glm::vec2 originsBB[4] = {glm::vec2(0, 0), glm::vec2(0, gridSize.y), glm::vec2(gridSize.x, gridSize.y), glm::vec2(gridSize.x, 0)};
    glm::vec2 endpointsBB[4] = {originsBB[1], originsBB[2], originsBB[3], originsBB[0]};

    for (int i = 0; i < N_RAYS; i++) {
        glm::vec2 origin = originFix + glm::vec2(rand()/(float)RAND_MAX,rand()/(float)RAND_MAX);
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
        glm::vec2 currentCell((int)(startingPoint.x), (int)(startingPoint.y));
        bool right = dir.x > 0;
        bool up = dir.y > 0;
        float deltaRemX = delta.x;
        float deltaRemY = delta.y;

        // bool addBelow = false;
        // bool addOnX = false;
        if (firstSide == 0 || firstSide == 2) {
            if (up)
                deltaRemY = (ceil(startingPoint.y) - startingPoint.y) * delta.y;
            else
                deltaRemY = (startingPoint.y - floor(startingPoint.y)) * delta.y;

            // if (abs(deltaRemY * cos(angle)) > 0.5) addBelow = true;
        } else if (firstSide == 1 || firstSide == 3) {
            if (right)
                deltaRemX = (ceil(startingPoint.x) - startingPoint.x) * delta.x;
            else
                deltaRemX = (startingPoint.x - floor(startingPoint.x)) * delta.x;

            // addOnX = true;
            // if (abs(deltaRemX * sin(angle)) > 0.5) addBelow = true; 
        }

        std::vector<glm::vec2> visiblePath;
        bool isPrevFloor = true;
        while (currentCell.x < gridSize.x && currentCell.x >= 0 && currentCell.y < gridSize.y && currentCell.y >= 0) {
            int y = (int)currentCell.x;
            int x = (int)currentCell.y;
            bool isCurrFloor = tilemap[x][y] != Tile::NOTHING;

            // Two contiguous cells are visible (both floor or outside)
            if ((isCurrFloor && isPrevFloor) || (!isCurrFloor && !isPrevFloor)) {
                for (glm::vec2 pathCell : visiblePath) {
                    visibilityPerCell[x][y].insert(pathCell);
                    visibilityPerCell[(int)pathCell.y][(int)pathCell.x].insert(currentCell);

                    // if (addOnX) {
                    //     if (abs(deltaRemX * sin(angle)) > 0.5) addBelow = true; 
                    //     else addBelow = false;
                    //     if (addBelow) {
                    //         if (x+1 < gridSize.y) {
                    //             visibilityPerCell[x+1][y].insert(pathCell);
                    //             visibilityPerCell[(int)pathCell.y][(int)pathCell.x].insert(glm::vec2(currentCell.x+1, currentCell.y));
                    //         }
                    //     } else {
                    //         if (x-1 >= 0) {
                    //             visibilityPerCell[x-1][y].insert(pathCell);
                    //             visibilityPerCell[(int)pathCell.y][(int)pathCell.x].insert(glm::vec2(currentCell.x-1, currentCell.y));
                    //         }
                    //     }
                    // } else {
                    //     if (abs(deltaRemX * cos(angle)) < 0.5) addBelow = true; 
                    //     else addBelow = false;
                    //     if (addBelow) {
                    //         if (y+1 < gridSize.x) {
                    //             visibilityPerCell[x][y+1].insert(pathCell);
                    //             visibilityPerCell[(int)pathCell.y][(int)pathCell.x].insert(glm::vec2(currentCell.x, currentCell.y+1));
                    //         }
                    //     } else {
                    //         if (y-1 >= 0) {
                    //             visibilityPerCell[x][y-1].insert(pathCell);
                    //             visibilityPerCell[(int)pathCell.y][(int)pathCell.x].insert(glm::vec2(currentCell.x, currentCell.y-1));
                    //         }
                    //     }
                    // }
                }
            } else {
                visiblePath.clear();
            }

            isPrevFloor = isCurrFloor;
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
            // if (deltaRemX > deltaRemY) addOnX = true;
            // else addOnX = false;
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

bool VisibilityComputer::writeVisibilityFile(glm::vec2 gridSize) 
{
    ofstream fout;
    fout.open("visibility.vis");
	if(!fout.is_open())
		return false;
    
    for (int i=0; i<gridSize.y; i++) {
        for (int j=0; j<gridSize.x; j++) {
            for (glm::vec2 cell : visibilityPerCell[i][j]) {
                fout << cell.x << " " << cell.y << " ";
            }
            fout << "\n";
        }
    }

    fout.close();

    return true;
}
