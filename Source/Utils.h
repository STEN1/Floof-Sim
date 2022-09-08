#pragma once
#include "Vertex.h"
#include <vector>
#include <string>


namespace FLOOF {
	namespace Utils {
		std::vector<Vertex> GetVisimVertexData(const std::string& path);
        std::vector<Vertex> MakeBall(int recursions, int radius);
		glm::vec3 CalcBarycentric(glm::vec3 position, const Triangle& triangle);
    }
}