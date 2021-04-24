
#include "Octree.h"
#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>


Octree::Octree(int maxDepth, glm::vec3 minAABB, glm::vec3 maxAABB, Octree *parent)
{
    this->maxDepth = maxDepth;
    this->minAABB = minAABB;
    this->maxAABB = maxAABB;

    this->parent = parent;

    for (int i=0; i<8; i++)
    {
        childs[i] = nullptr;
    }

    meanPosition = glm::vec3(0.0f);
    nVertices = 0;
}

Octree::~Octree()
{
    for (int i=0; i<8; i++) 
        if (childs[i])
            delete childs[i];

    // delete this;
}

glm::vec3 Octree::getAvgPosition()
{
    return meanPosition;
}

Octree* Octree::evaluateVertex(glm::vec3 vertex)
{
    // add vertex contribution
    meanPosition = (meanPosition * (float)nVertices + vertex) / ((float)(nVertices+1));
    nVertices++;
    
    // check if has reached maximum depth
    if (maxDepth == 0)
        return this;

    glm::vec3 extents = maxAABB - minAABB;

    // assign vertex to its corresponding subtree
    bool x = vertex.x > (minAABB.x + extents.x / 2.0f); // Is at right half
    bool y = vertex.y > (minAABB.y + extents.y / 2.0f); // Is at top half
    bool z = vertex.z > (minAABB.z + extents.z / 2.0f); // Is at front half
    
    if (!childs[z * 4 + y * 2 + x])
    {
        glm::vec3 minAABBchild = minAABB + glm::vec3(x*extents.x/2.0f, y*extents.y/2.0f, z*extents.z/2.0f);
        glm::vec3 maxAABBchild = minAABB + extents/2.0f + glm::vec3(x*extents.x/2.0f, y*extents.y/2.0f, z*extents.z/2.0f);
        childs[z * 4 + y * 2 + x] = new Octree(maxDepth-1, minAABBchild, maxAABBchild, this); 
    }

    return childs[z * 4 + y * 2 + x]->evaluateVertex(vertex);
}
