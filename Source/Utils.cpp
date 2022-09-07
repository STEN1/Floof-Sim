#include "Utils.h"

#include "Floof.h"

#include <fstream>

static void SubDivide(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, int recursions, std::vector<FLOOF::Vertex> &vertexData);

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

        std::vector<Vertex> MakeBall(int recursions, int radius) {
            int r = radius;
            std::vector<Vertex> vertexData;

            glm::vec3 v0{0,0,r};
            glm::vec3 v1{r,0,0};
            glm::vec3 v2{0,r,0};
            glm::vec3 v3{-r,0,0};
            glm::vec3 v4{0,-r,0};
            glm::vec3 v5{0,0,-r};

            SubDivide(v0, v1, v2, recursions,vertexData);
            SubDivide(v0, v2, v3, recursions,vertexData);
            SubDivide(v0, v3, v4, recursions,vertexData);
            SubDivide(v0, v4, v1, recursions,vertexData);
            SubDivide(v5, v2, v1, recursions,vertexData);
            SubDivide(v5, v3, v2, recursions,vertexData);
            SubDivide(v5, v4, v3, recursions,vertexData);
            SubDivide(v5, v1, v4, recursions,vertexData);

            return { vertexData };
        }
    }
}

static void SubDivide(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, int recursions, std::vector<FLOOF::Vertex> &vertexData) {
    if (recursions > 0) {
        glm::vec3 v1 = glm::normalize(a + b);
        glm::vec3 v2 = glm::normalize(a + c);
        glm::vec3 v3 = glm::normalize(c + b);

        SubDivide(a, v1, v2, recursions - 1, vertexData);
        SubDivide(c, v2, v3, recursions - 1, vertexData);
        SubDivide(b, v3, v1, recursions - 1, vertexData);
        SubDivide(v3, v2, v1, recursions - 1, vertexData);
    } else {
        FLOOF::Vertex v{};
        // TODO: Calculate normals
        v.pos.x = a.x;
        v.pos.y = a.y;
        v.pos.z = a.z;
        vertexData.push_back(v);
        v.pos.x = b.x;
        v.pos.y = b.y;
        v.pos.z = b.z;
        vertexData.push_back(v);
        v.pos.x = c.x;
        v.pos.y = c.y;
        v.pos.z = c.z;
        vertexData.push_back(v);
    }
}

