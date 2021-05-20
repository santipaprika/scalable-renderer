#ifndef __NODE_H__
#define __NODE_H__

#include <glm/glm.hpp>
#include "TriangleMesh.h"

class Node
{
private:
    glm::mat4 model;
    TriangleMesh *mesh;


public:
    Node(TriangleMesh *mesh, glm::mat4 model = glm::mat4(1.0));
    ~Node();

    glm::mat4 &getModel();
    TriangleMesh *getMesh();
    glm::vec3 getPosition();
    
    void usePreviousLod();
    void useNextLod();
};

#endif // __NODE_H__