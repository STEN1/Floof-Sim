#pragma once

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtx/quaternion.hpp"
#include <random>

namespace FLOOF{
    namespace Math{
        constexpr inline const double Gravity{9.807};
        static const glm::vec3 GravitationalAcceleration(0.f,-Gravity,0.f);
        static size_t Cantor(size_t a, size_t b) { return (a + b + 1) * (a + b) / 2 + b; }


        static std::random_device RandomDevice;
        static std::mt19937_64 Generator(RandomDevice());

        static double RandDouble(const double min, const double max){
            std::uniform_real_distribution<>dist(min,max);
            return dist(Generator);
        }
        static int RandInt(const int min, const int max) {
            std::uniform_int_distribution<> dist(min, max);
            return dist(Generator);
        }
    }
}
