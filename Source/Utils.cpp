#include "Utils.h"

#include "Floof.h"

#include <fstream>

namespace FLOOF {
	namespace Utils {
		std::pair<std::vector<Vertex>, std::vector<uint32_t>> GetVisimVertexData(const std::string& path) {
			std::vector<Vertex> vertexData;
			std::vector<uint32_t> indexData;

			std::ifstream file(path);
			if (!file.is_open()) {
				std::cout << "Cant open file: " << path << std::endl;
				return { vertexData, indexData };
			}

			uint32_t vertexCount{}; // Throwaway variabel.
			file >> vertexCount;

			while (!file.eof()) {
				Vertex v{};
				file >> v.pos.x;
				file >> v.pos.y;
				file >> v.pos.z;

				vertexData.push_back(v);
			}

			return { vertexData, indexData };
		}
	}
}
