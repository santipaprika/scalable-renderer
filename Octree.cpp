
#include "Octree.h"
#include <iostream>

Octree::Octree(vector<glm::vec3> vertices, int depth, glm::vec3 minAABB, glm::vec3 maxAABB, Octree *parent)
{
    // Assumption: all vertices introduced as argument fall inside the octree bounds [minAABB, maxAABB].

    // Assign parent
    this->parent = parent;
    if (parent)
        nodeDepth = parent->nodeDepth + 1;
    else
        nodeDepth = 0;

    meanPosition = glm::vec3(0.f);
    // Compute mean vertex position
    for (glm::vec3 vert : vertices)
        meanPosition += vert;

    meanPosition /= (float)vertices.size();

    // assign AABB
    this->minAABB = minAABB;
    this->maxAABB = maxAABB;

    // end if it reaches max depth
    if (depth == 0)
        return;

    // compute AABB minimum for the 8 childs
    vector<glm::vec3> children_vertices[8];
    glm::vec3 extents = maxAABB - minAABB;
    glm::vec3 children_minAABB[8] = {
        minAABB, minAABB + glm::vec3(extents.x / 2.0f, 0, 0),                                                                                 // BOTTOM - BACK
        minAABB + glm::vec3(0, extents.y / 2.0f, 0), minAABB + glm::vec3(extents.x / 2.0f, extents.y / 2.0f, 0),                              // TOP - BACK
        minAABB + glm::vec3(0, 0, extents.z / 2.0f), minAABB + glm::vec3(extents.x / 2.0f, 0, extents.z / 2.0f),                              // BOTTOM - FRONT
        minAABB + glm::vec3(0, extents.y / 2.0f, extents.z / 2.0f), minAABB + glm::vec3(extents.x / 2.0f, extents.y / 2.0f, extents.z / 2.0f) // TOP - FRONT
    };
    glm::vec3 children_maxAABB[8];

    // assign each vertex to its corresponding subtree
    for (glm::vec3 vertex : vertices)
    {
        bool x = vertex.x > (minAABB.x + extents.x / 2.0f); // Is at right half
        bool y = vertex.y > (minAABB.y + extents.y / 2.0f); // Is at top half
        bool z = vertex.z > (minAABB.z + extents.z / 2.0f); // Is at front half

        children_vertices[z * 4 + y * 2 + x].push_back(vertex);
    }

    // Create childs
    for (int i = 0; i < 8; i++)
    {
        if (!parent)
            printf("[ %6.2f%%] Generating LODs octree (child %d).\n", i / 8.f * 100, i);

        // nodes with less than 8 children may arise, but build phase is much more efficient!
        if (children_vertices[i].size() > 0)
            childs[i] = new Octree(children_vertices[i], depth - 1, children_minAABB[i], children_minAABB[i] + extents / 2.0f, this);
        else
            childs[i] = nullptr;
    }

    if (!parent)
        printf("[ %6.2f%%] Done!\n", 100.f);
}

Octree::~Octree()
{
    for (Octree *child : childs)
    {
        delete child;
    }
}

glm::vec3 Octree::getAvgPosition()
{
    return meanPosition;
}

bool Octree::isVertexContained(glm::vec3 vertex)
{
    bool biggerThanMin = vertex.x > minAABB.x && vertex.y > minAABB.y && vertex.z > minAABB.z;
    bool smallerThanMax = vertex.x < maxAABB.x && vertex.y < maxAABB.y && vertex.z < maxAABB.z;

    return biggerThanMin && smallerThanMax;
}

void Octree::getOctreesAtDepth(std::vector<Octree *> &levelOctrees, int depth)
{
    if (nodeDepth == depth)
    {
        levelOctrees.push_back(this);
        return;
    }

    for (int i = 0; i < 8; i++)
    {
        if (childs[i])
            childs[i]->getOctreesAtDepth(levelOctrees, depth);
    }
}
