

#ifndef FLOOF_PHYSICS_H
#define FLOOF_PHYSICS_H
#include "glm/glm.hpp"

namespace FLOOF{
    struct BallComponent;
    struct Triangle;

    class Physics {
    public:
        Physics() = delete;

    static bool TriangleBallIntersect(const Triangle &triangle, const glm::vec3 &position, const float &radius);
    static glm::vec3 GetVelocityBall(const Triangle & triangle, const glm::vec3 & position, const BallComponent& ball, const glm::vec3 &Velocity, const double & deltatime);
    };
}



#endif //FLOOF_PHYSICS_H
