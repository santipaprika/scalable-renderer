#ifndef __NODE_H__
#define __NODE_H__

#include <glm/glm.hpp>

#include "TriangleMesh.h"

class Node {
   private:
    glm::mat4 model;
    TriangleMesh *mesh;
    float benefit;
    float value;

   public:
    Node(TriangleMesh *mesh, glm::mat4 model = glm::mat4(1.0));
    ~Node();

    glm::mat4 &getModel();
    TriangleMesh *getMesh() const;
    glm::vec3 getPosition();

    void usePreviousLod();
    void useNextLod();
    void useLowestLod();

    float getBenefit() const;
    void setBenefit(float benefit);
    void computeBenefit(glm::vec3 viewpoint);

};

struct NodePtrLess {
    bool operator()(const Node* n1, const Node* n2) {
        return (n1->getBenefit() / n1->getMesh()->getCost()) <= (n2->getBenefit() / n2->getMesh()->getCost());
    }
};

#endif  // __NODE_H__