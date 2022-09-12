

#ifndef FLOOF_PHYSICS_H
#define FLOOF_PHYSICS_H
#include "../Vertex.h"

namespace FLOOF{
    class Physics {
    public:
        Physics() = delete;

    static bool TriangleBallIntersect(const Triangle &triangle, const glm::vec3 &position, const float &radius);

    };
}



#endif //FLOOF_PHYSICS_H
