#include "Physics.h"
#include "cmath"

bool FLOOF::Physics::TriangleBallIntersect(const FLOOF::Triangle &triangle, const glm::vec3 &position, const float &radius) {
   auto intersect = std::abs(glm::dot(position-triangle.A,triangle.N));
   return (intersect > radius);
}

glm::vec3
FLOOF::Physics::moveBallOnTriangle(const FLOOF::Triangle &triangle, const glm::vec3 &position, const float &radius, const glm::vec3 &velocity) {

    glm::vec3 G(0.0,Math::Gravity,0.0);
    glm::vec3 N = -G;

    glm::vec3 test = glm::reflect(G, triangle.N);

    return test;
}


