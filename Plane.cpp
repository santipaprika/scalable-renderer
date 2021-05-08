#include "Plane.h"


Plane::Plane(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3) 
{
    glm::vec3 v1 = glm::normalize(p2-p1);
    glm::vec3 v2 = glm::normalize(p3-p1);

    glm::vec3 n = glm::normalize(glm::cross(v1,v2));

    a = n.x;
    b = n.y;
    c = n.z;
    d = glm::dot(p1,n);
}

Plane::~Plane()
{
}