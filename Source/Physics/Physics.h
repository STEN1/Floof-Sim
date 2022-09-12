

#ifndef FLOOF_PHYSICS_H
#define FLOOF_PHYSICS_H
#include "../Vertex.h"

namespace FLOOF{
    class Physics {
    public:
        Physics() = delete;

    static bool TriangleBallIntersect(const Triangle &triangle, const glm::vec3 &position, const float &radius);
    static glm::vec3 GetVelocityBall(const Triangle & triangle, const glm::vec3 & position, const float & radius, const glm::vec3 &Velocity);
    };
}



#endif //FLOOF_PHYSICS_H
