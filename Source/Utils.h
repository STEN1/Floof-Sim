#pragma once
#include "Vertex.h"
#include <vector>
#include <string>


namespace FLOOF {
	namespace Utils {
		std::vector<MeshVertex> GetVisimVertexData(const std::string& path);
        std::vector<MeshVertex> MakeBall(int subdivisions, float radius);
		glm::vec3 CalcBarycentric(glm::vec3 position, const Triangle& triangle);
        bool isInside(const glm::vec3 & position, const Triangle & triangle);
    }
}