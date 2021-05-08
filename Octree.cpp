
#include "Octree.h"

#include <iostream>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

Octree::Octree(int maxDepth, glm::vec3 minAABB, float halfLength, Octree *parent) {
    this->maxDepth = maxDepth;
    this->minAABB = minAABB;
    this->halfLength = halfLength;

    this->parent = parent;

    for (int i = 0; i < 8; i++) {
        childs[i] = nullptr;
    }

    position = glm::vec3(0.0f);
    addedPosition = glm::vec3(0.0f);
    nVertices = 0;

    idx = counter;
    counter++;
}

int Octree::counter;

Octree::~Octree() {
    for (int i = 0; i < 8; i++)
        if (childs[i])
            delete childs[i];

    // delete this;
}

glm::vec3 Octree::getPosition() const {
    return position;
}

Octree *Octree::evaluateVertex(const glm::vec3 &vertex, unordered_map<int, unordered_set<Plane *>> &vertexToQuadric, int idx) {
    // add vertex contribution
    addedPosition += vertex;
    for (Plane *plane : vertexToQuadric[idx]) {
        quadrics.insert(plane);
    }

    nVertices++;

    // check if has reached maximum depth
    if (maxDepth == 0)
        return this;

    // vertex vector = vertex - AABB center
    glm::vec3 vertexVec = vertex - (minAABB + halfLength);

    // assign vertex to its corresponding subtree
    bool x = vertexVec.x > 0;  // Is at right half
    bool y = vertexVec.y > 0;  // Is at top half
    bool z = vertexVec.z > 0;  // Is at front half

    if (!childs[z * 4 + y * 2 + x]) {
        glm::vec3 minAABBchild = minAABB + glm::vec3(x, y, z) * halfLength;
        childs[z * 4 + y * 2 + x] = new Octree(maxDepth - 1, minAABBchild, halfLength / 2.0f, this);
    }

    return childs[z * 4 + y * 2 + x]->evaluateVertex(vertex, vertexToQuadric, idx);
}

int Octree::getIndex() const {
    return idx;
}

void Octree::computeMeanPositions() {
    for (int i = 0; i < 8; i++) {
        if (childs[i])
            childs[i]->computeMeanPositions();
    }
    position = addedPosition / (float)nVertices;
}

void Octree::computeQEMPositions() {
    for (int i = 0; i < 8; i++) {
        if (childs[i])
            childs[i]->computeQEMPositions();
    }

    position = Plane::computePointMinimizingQEM(quadrics);
}
