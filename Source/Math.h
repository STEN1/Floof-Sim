#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/quaternion.hpp"

namespace FLOOF{
    namespace Math{
        constexpr inline const double Gravity{9.807};
        static size_t Cantor(size_t a, size_t b) { return (a + b + 1) * (a + b) / 2 + b; }
    }
}
