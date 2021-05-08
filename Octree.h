#ifndef __OCTREE_H__
#define __OCTREE_H__

#include <unordered_set>
#include <unordered_map>
#include "glm/glm.hpp"
#include "Plane.h"

using namespace std;

class Octree
{
private:
    glm::vec3 minAABB;
    float halfLength;

    glm::vec3 addedPosition;
    glm::vec3 position;
    int maxDepth;
    int nVertices;
    int idx;
    unordered_set<Plane*> quadrics;

    static int counter;

public:
    // Octree(vector<glm::vec3> vertices, int depth, glm::vec3 minAABB, glm::vec3 maxAABB, Octree *parent = nullptr);
    Octree(int maxDepth, glm::vec3 minAABB, float halfLength, Octree *parent = nullptr);
    ~Octree();

    glm::vec3 getPosition() const;
    Octree* evaluateVertex(const glm::vec3 &vertex, unordered_map<int, unordered_set<Plane *>> &vertexToQuadric, int idx=0);
    int getIndex() const;
    void computeMeanPositions();
    void computeQEMPositions();

    Octree *childs[8];
    Octree *parent;
};

#endif // __OCTREE_H__