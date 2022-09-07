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

			std::vector<float> tempVertexData;
			while (!file.eof()) {
				float temp;
				file >> temp;
				tempVertexData.push_back(temp);
			}

			for (uint32_t i = 2; i < tempVertexData.size(); i += 3) {
				Vertex v{};
				v.pos.z = tempVertexData[i - 2];
				v.pos.y = tempVertexData[i - 1];
				v.pos.x = tempVertexData[i - 0];
			}

			return { vertexData, indexData };
		}
	}
}
