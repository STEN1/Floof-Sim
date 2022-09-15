

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
    static glm::vec3 GetReflectVelocity(const glm::vec3 & velocity, const glm::vec3 & reflectionangle);
    static glm::vec3 GetReflectionAngle(const glm::vec3 & m, const glm::vec3 & n);
    static glm::vec3 GetAccelerationVector(const Triangle& triangle);
    };
}



#endif //FLOOF_PHYSICS_H
