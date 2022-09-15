#include "Physics.h"
#include "cmath"
#include "../Components.h"
#include "../Vertex.h"
#include "../LoggerMacros.h"

bool FLOOF::Physics::TriangleBallIntersect(const FLOOF::Triangle &triangle, const glm::vec3 &position, const float &radius) {
   auto intersect = (glm::dot(position-triangle.A,triangle.N));
   return (intersect < radius);
}

glm::vec3 FLOOF::Physics::GetReflectVelocity(const glm::vec3 &velocity, const glm::vec3 &reflectionAngle) {
    return velocity-(2.f*(velocity*reflectionAngle)*reflectionAngle);
}

glm::vec3 FLOOF::Physics::GetReflectionAngle(const glm::vec3 &m, const glm::vec3 &n) {
    return glm::normalize((m + n) / (glm::length(m + n) * glm::length(m + n)));
}

glm::vec3 FLOOF::Physics::GetAccelerationVector(const FLOOF::Triangle &triangle) {
    return (float(Math::Gravity) * glm::vec3(triangle.N.x * triangle.N.y, (triangle.N.y * triangle.N.y) - 1,(triangle.N.z * triangle.N.y)));
}
