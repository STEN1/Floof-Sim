#include "Utils.h"

#include "Floof.h"

#include <fstream>

namespace FLOOF {
	namespace Utils {
		std::vector<Vertex> GetVisimVertexData(const std::string& path) {
			std::vector<Vertex> vertexData;

			std::ifstream file(path);
			if (!file.is_open()) {
				std::cout << "Cant open file: " << path << std::endl;
				return vertexData;
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
				v.pos.x = tempVertexData[i - 2];
				v.pos.y = tempVertexData[i - 1];
				v.pos.z = tempVertexData[i - 0];
				vertexData.push_back(v);
			}

			for (uint32_t i = 2; i < vertexData.size(); i += 3) {
				Vertex& a = vertexData[i - 2];
				Vertex& b = vertexData[i - 1];
				Vertex& c = vertexData[i - 0];

				glm::vec3 ab = b.pos - a.pos;
				glm::vec3 ac = c.pos - a.pos;

				glm::vec3 normal = glm::normalize(glm::cross(ab, ac));

				a.normal = normal;
				b.normal = normal;
				c.normal = normal;

				a.uv.x = a.pos.x;
				a.uv.y = a.pos.y;
				b.uv.x = b.pos.x;
				b.uv.y = b.pos.y;
				c.uv.x = c.pos.x;
				c.uv.y = c.pos.y;
			}

			return vertexData;
		}
	}
}
