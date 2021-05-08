#ifndef __PLANE_H__
#define __PLANE_H__

#include <glm/glm.hpp>

class Plane
{
private:
    float a,b,c,d;
public:
    Plane(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3);
    ~Plane();
};

#endif // __PLANE_H__