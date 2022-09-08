#pragma once
#include "Vertex.h"
#include <vector>
#include <string>


namespace FLOOF {
	namespace Utils {
		std::pair<std::vector<Vertex>, std::vector<uint32_t>> GetVisimVertexData(const std::string& path);
        std::vector<Vertex> MakeBall(int recursions, int radius);
		static glm::vec3 CalcBarycentric(glm::vec3 position, const Triangle& triangle);
    }
}