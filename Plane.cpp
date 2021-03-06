#include "Plane.h"

#include <iostream>
#include <Eigen/SVD>

using namespace Eigen;

Plane::Plane(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3) {
    glm::highp_vec3 v1 = glm::normalize(p2 - p1);
    glm::highp_vec3 v2 = glm::normalize(p3 - p1);

    // Left hand
    glm::highp_vec3 n = glm::normalize(glm::cross(v1, v2));

    a = n.x;
    b = n.y;
    c = n.z;
    d = glm::dot(p1, n);
}

Plane::~Plane() {
}

glm::vec3 Plane::getNormal() 
{
    return glm::vec3(a,b,c);
}

Vector4d Plane::getPlaneParams() 
{
    return Vector4d(a,b,c,d);
}

glm::vec3 Plane::computePointMinimizingQEM(Matrix4d Q) {
    Q(3,0) = 0.0;
    Q(3,1) = 0.0;
    Q(3,2) = 0.0;
    Q(3,3) = 1.0;

    JacobiSVD<Matrix4d> svd(Q, ComputeFullU | ComputeFullV);
    Vector4d rhs(0,0,0,1);

    Vector4d sol = -svd.solve(rhs);

    return glm::vec3(sol[0], sol[1], sol[2]);
}