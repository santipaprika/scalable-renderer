
#include "Octree.h"

Octree::Octree(vector<glm::vec3> vertices)
{
    // Assumption: all vertices passed fall inside the octree.

    vector<glm::vec3> children_vertices[8];
    glm::vec3 diff_min_max = maxAABB - minAABB;
    glm::vec3 children_minAABB[8] = {
        minAABB, minAABB + glm::vec3(diff_min_max.x / 2.0f, 0, 0),  // BOTTOM - BACK
        minAABB + glm::vec3(0, diff_min_max.y / 2.0f, 0), minAABB + glm::vec3(diff_min_max.x / 2.0f, diff_min_max.y / 2.0f, 0),  // TOP - BACK
        minAABB + glm::vec3(0, 0, diff_min_max.z / 2.0f), minAABB + glm::vec3(diff_min_max.x / 2.0f, 0, diff_min_max.z / 2.0f),  // BOTTOM - FRONT
        minAABB + glm::vec3(0, diff_min_max.y / 2.0f, diff_min_max.z / 2.0f), minAABB + glm::vec3(diff_min_max.x / 2.0f, diff_min_max.y / 2.0f, diff_min_max.z / 2.0f)  // TOP - FRONT
    };
    glm::vec3 children_maxAABB[8];

    for (glm::vec3 vertex : vertices)
    {
        if (vertex.x < minAABB.x + diff_min_max.x/2.0f) // 
    }
}

Octree::~Octree()
{
    for (Octree *child : childs)
    {
        delete child;
    }
}
