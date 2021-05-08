#ifndef __PLANE_H__
#define __PLANE_H__

#include <glm/glm.hpp>
#include <unordered_set>

class Plane
{
private:
    double a,b,c,d;
public:
    Plane(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3);
    ~Plane();

    static glm::vec3 computePointMinimizingQEM(std::unordered_set<Plane*> planes);
};

#endif // __PLANE_H__