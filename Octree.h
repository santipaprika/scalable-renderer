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
    int nodeDepth;

public:
    Octree(vector<glm::vec3> vertices, int depth, glm::vec3 minAABB, glm::vec3 maxAABB, Octree *parent = nullptr);
    ~Octree();

    glm::vec3 getAvgPosition();
    bool isVertexContained(glm::vec3 vertex);
    void getOctreesAtDepth(std::vector<Octree *> &levelOctrees, int depth);

    Octree *childs[8];
    Octree *parent;
};

#endif // __OCTREE_H__