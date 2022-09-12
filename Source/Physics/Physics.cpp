#include "Physics.h"
#include "cmath"

bool FLOOF::Physics::TriangleBallIntersect(const FLOOF::Triangle &triangle, const glm::vec3 &position, const float &radius) {
   auto intersect = std::abs(glm::dot(position-triangle.A,triangle.N));
   return (intersect > radius);
}
