#ifndef __OCTREE_H__
#define __OCTREE_H__

#include <vector>
#include "glm/glm.hpp"

using namespace std;

class Octree
{
private:
    glm::vec3 minAABB;
    glm::vec3 maxAABB;

    glm::vec3 meanPosition;
    int maxDepth;
    int nVertices;

public:
    // Octree(vector<glm::vec3> vertices, int depth, glm::vec3 minAABB, glm::vec3 maxAABB, Octree *parent = nullptr);
    Octree(int maxDepth, glm::vec3 minAABB, glm::vec3 maxAABB, Octree *parent = nullptr);
    ~Octree();

    glm::vec3 getAvgPosition();
    Octree* evaluateVertex(glm::vec3 vertex);

    Octree *childs[8];
    Octree *parent;
};

#endif // __OCTREE_H__