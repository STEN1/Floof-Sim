#include "Physics.h"
#include "cmath"
#include "../Components.h"
#include "../Vertex.h"

bool FLOOF::Physics::TriangleBallIntersect(const FLOOF::Triangle &triangle, const glm::vec3 &position, const float &radius) {
   auto intersect = std::abs(glm::dot(position-triangle.A,triangle.N));
   return (intersect > radius);
}

glm::vec3
FLOOF::Physics::GetVelocityBall(const FLOOF::Triangle &triangle, const glm::vec3 &position, const BallComponent &ball, const glm::vec3 &velocity) {

    //calculate magic so old velocity matters
    glm::vec3 G(0.0,Math::Gravity,0.0);
    glm::vec3 N = -G;
    // TODO fix velocity calculations
    // F = mv/t
    //calculate reflect between ball velocity and triangle
    glm::vec3 test = glm::reflect(G, triangle.N);
    test += velocity;
    //return new velocity
    return test;
}


