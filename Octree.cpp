
#include "Octree.h"
#include <iostream>

Octree::Octree(vector<glm::vec3> vertices, int depth, glm::vec3 minAABB, glm::vec3 maxAABB, Octree* parent)
{
    // Assumption: all vertices passed fall inside the octree.

    // Assign parent
    this->parent = parent;

    // Compute mean vertex position
    for (glm::vec3 vert : vertices)
        meanPosition += vert;

    meanPosition /= (float)vertices.size();

    // assign AABB
    this->minAABB = minAABB;
    this->maxAABB = maxAABB;

    // end if it reaches max depth
    if (depth == 0) return;

    // compute AABB minimum for the 8 childs
    vector<glm::vec3> children_vertices[8];
    glm::vec3 extents = maxAABB - minAABB;
    glm::vec3 children_minAABB[8] = {
        minAABB, minAABB + glm::vec3(extents.x / 2.0f, 0, 0),  // BOTTOM - BACK
        minAABB + glm::vec3(0, extents.y / 2.0f, 0), minAABB + glm::vec3(extents.x / 2.0f, extents.y / 2.0f, 0),  // TOP - BACK
        minAABB + glm::vec3(0, 0, extents.z / 2.0f), minAABB + glm::vec3(extents.x / 2.0f, 0, extents.z / 2.0f),  // BOTTOM - FRONT
        minAABB + glm::vec3(0, extents.y / 2.0f, extents.z / 2.0f), minAABB + glm::vec3(extents.x / 2.0f, extents.y / 2.0f, extents.z / 2.0f)  // TOP - FRONT
    };
    glm::vec3 children_maxAABB[8];

    // assign each vertex to its corresponding subtree
    for (glm::vec3 vertex : vertices)
    {
        bool x = vertex.x > (minAABB.x + extents.x/2.0f);  // Is at right half
        bool y = vertex.y > (minAABB.y + extents.y/2.0f);  // Is at top half
        bool z = vertex.z > (minAABB.z + extents.z/2.0f);  // Is at front half

        children_vertices[z*4 + y*2 + x].push_back(vertex);
    }

    // Create childs
    for (int i=0; i<8; i++)
    {
        if  (!parent)
            printf("[ %6.2f%%] Generating LODs octree (child %d).\n", i/8.f * 100, i);
            // std::cout << "[ " << i/8.f * 100 << "%] Generating LODs." << std::endl; 
        childs[i] = new Octree(children_vertices[i], depth-1, children_minAABB[i], children_minAABB[i] + extents/2.0f, this);
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
