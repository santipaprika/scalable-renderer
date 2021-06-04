#include "Utils.h"
#include <algorithm>
#include <regex>
#include <iostream>

float Utils::max3(glm::vec3 v)
{
    return std::max(v.x, std::max(v.y, v.z));
}

glm::vec2 Utils::getGridSize(std::ifstream &tilemap)
{
    // get first 5 lines (headers)
    std::string lineStr;
    glm::vec2 gridSize;
    for (int i = 0; i < 5; i++)
    {
        std::getline(tilemap, lineStr);

        // parse 3rd line (which contiains width/height information)
        if (i == 3)
        {
            std::string word;
            std::stringstream lineSS = std::stringstream(lineStr);
            while (!lineSS.eof())
            {
                lineSS >> word;
                if (word.find("width=") != std::string::npos)
                {
                    gridSize.x = stoi(std::regex_replace(word, std::regex("[^0-9]*([0-9]+).*"), std::string("$1")));
                }
                if (word.find("height=") != std::string::npos)
                {
                    gridSize.y = stoi(std::regex_replace(word, std::regex("[^0-9]*([0-9]+).*"), std::string("$1")));
                }
            }
        }
    }

    return gridSize;
}

void Utils::parseGrid(std::ifstream &tilemap, int **&grid, glm::vec2 gridSize)
{
    for (int i = 0; i < gridSize.y; i++)
    {
        std::string lineStr;

        std::getline(tilemap, lineStr);

        std::stringstream ss(lineStr);
        int num_label;
        for (int j = 0; j < gridSize.x; j++)
        {
            ss >> num_label;
            grid[i][j] = num_label-1;
            if (ss.peek() == ',')
                ss.ignore();
        }
    }
}

// template<typename T>
// void Utils::initializePointerMatrix(T** &pointerArray, int sizeX, int sizeY) 


void Utils::deletePointerMatrix(int** &pointerArray, int sizeX, int sizeY) 
{
    for (int i = 0; i < sizeY; i++)
        delete[] pointerArray[i];
    delete[] pointerArray;
}
