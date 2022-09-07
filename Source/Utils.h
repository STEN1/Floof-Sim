#pragma once
#include "Vertex.h"
#include <vector>
#include <string>


namespace FLOOF {
	namespace Utils {
		std::vector<Vertex> GetVisimVertexData(const std::string& path);
	}
}