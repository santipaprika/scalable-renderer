#ifndef __PLANE_H__
#define __PLANE_H__

#include <glm/glm.hpp>
#include <Eigen/Core>

class Plane
{
private:
    double a,b,c,d;
public:
    Plane(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3);
    ~Plane();

    glm::vec3 getNormal(); 
    Eigen::Vector4d getPlaneParams();

    static glm::vec3 computePointMinimizingQEM(Eigen::Matrix4d Q);
};

#endif // __PLANE_H__