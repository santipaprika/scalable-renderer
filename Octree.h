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

    glm::vec3 vertex;
    Octree *childs[8];
    Octree *parent;
    
public:
    Octree(vector<glm::vec3> vertices);
    ~Octree();
};

#endif // __OCTREE_H__