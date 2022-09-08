#include "Utils.h"

#include "Floof.h"

#include <fstream>

static void SubDivide(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, int recursions, std::vector<FLOOF::Vertex> &vertexData);
static void pushVerts(glm::vec3 a, glm::vec3 b, glm::vec3 c, std::vector<FLOOF::Vertex> &vertexData);
static glm::vec3 calcBarycentric(glm::vec3 position, glm::vec3 a, glm::vec3 b, glm::vec2 c);

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

			uint32_t vertexCount{};
			file >> vertexCount;
			uint32_t indexCount{};
			file >> indexCount;

			vertexData.resize(vertexCount);
			for (Vertex& vertex : vertexData) {
				file >> vertex.pos.x;
				file >> vertex.pos.y;
				file >> vertex.pos.z;
			}

			indexData.resize(indexCount);
			for (uint32_t& i : indexData) {
				file >> i;
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

        pushVerts(a,b,c,vertexData);
    }
}

static void pushVerts(glm::vec3 a, glm::vec3 b, glm::vec3 c, std::vector<FLOOF::Vertex> &vertexData) {

    glm::vec3 normal = glm::cross(b - a, c - a);
    normal = glm::normalize(normal);
    FLOOF::Vertex v{};

    v.pos = a;
    v.normal = normal;
    vertexData.push_back(v);

    FLOOF::Vertex vertB{};
    v.pos = b;
    v.normal = normal;
    vertexData.push_back(v);

    FLOOF::Vertex vertC{};
    v.pos = c;
    v.normal = normal;
    vertexData.push_back(v);
}

static glm::vec3 calcBarycentric(glm::vec3 position, glm::vec3 a, glm::vec3 b, glm::vec2 c) {

    glm::vec2 pos{position.x,position.y};
    
    glm::vec2 p1{a.x,a.y};
    glm::vec2 p2{b.x,b.y};
    glm::vec2 p3{c.x,c.y};

    glm::vec2 Va = p2 - p1;
    glm::vec2 Vb = p3 - p1;
    glm::vec3 n = glm::cross(glm::vec3(Va.x, Va.y, 0), glm::vec3(Vb.x, Vb.y, 0));
    float area = glm::length(n);
    glm::vec3 cords;

    glm::vec2 p = p2 - pos;
    glm::vec2 q = p3 - pos;
    n = glm::cross(glm::vec3(p.x, p.y, 0), glm::vec3(q.x, q.y, 0));
    cords.x = n.z / area;

    p = p3 - pos;
    q = p1 - pos;
    n = glm::cross(glm::vec3(p.x, p.y, 0), glm::vec3(q.x, q.y, 0));
    cords.y = n.z / area;

    p = p1 - pos;
    q = p2 - pos;
    n = glm::cross(glm::vec3(p.x, p.y, 0), glm::vec3(q.x, q.y, 0));
    cords.z = n.z / area;

    return cords;
}
