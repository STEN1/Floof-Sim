#include "Utils.h"

#include "Floof.h"

#include <fstream>
#include <string>
#include <sstream>


namespace FLOOF {
	namespace Utils {
        static void SubDivide(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, int recursions, std::vector<FLOOF::MeshVertex> &vertexData);
        static void PushVerts(glm::vec3 a, glm::vec3 b, glm::vec3 c, std::vector<FLOOF::MeshVertex> &vertexData);

		std::vector<MeshVertex> GetVisimVertexData(const std::string& path) {
			std::vector<MeshVertex> vertexData;

			std::ifstream file(path);
			if (!file.is_open()) {
				std::cout << "Cant open file: " << path << std::endl;
                return vertexData;
			}

			uint32_t vertexCount{};
			file >> vertexCount;
            std::string s;
            std::getline(file, s);
			vertexData.resize(vertexCount);
			for (MeshVertex& vertex : vertexData) {
                std::string line;
                std::getline(file, line);
                std::stringstream ss(line);
				ss >> vertex.Pos.x;
				ss >> vertex.Pos.y;
				ss >> vertex.Pos.z;
			}

			for (uint32_t i = 2; i < vertexData.size(); i += 3) {
				MeshVertex& a = vertexData[i - 2];
				MeshVertex& b = vertexData[i - 1];
				MeshVertex& c = vertexData[i - 0];

				glm::vec3 ab = b.Pos - a.Pos;
				glm::vec3 ac = c.Pos - a.Pos;

                glm::vec3 normal = glm::normalize(glm::cross(ab, ac));

				a.Normal = normal;
				b.Normal = normal;
				c.Normal = normal;

				a.UV.x = a.Pos.x;
				a.UV.y = a.Pos.y;
				b.UV.x = b.Pos.x;
				b.UV.y = b.Pos.y;
				c.UV.x = c.Pos.x;
				c.UV.y = c.Pos.y;
			}

            return vertexData;
		}

        std::vector<MeshVertex> MakeBall(const int & density, const float & radius) {

            std::vector<MeshVertex> vertexData;

            glm::vec3 v0{0,0,radius};
            glm::vec3 v1{radius,0,0};
            glm::vec3 v2{0,radius,0};
            glm::vec3 v3{-radius,0,0};
            glm::vec3 v4{0,-radius,0};
            glm::vec3 v5{0,0,-radius};

            SubDivide(v0, v1, v2,density ,vertexData);
            SubDivide(v0, v2, v3, density,vertexData);
            SubDivide(v0, v3, v4, density,vertexData);
            SubDivide(v0, v4, v1, density,vertexData);
            SubDivide(v5, v2, v1, density,vertexData);
            SubDivide(v5, v3, v2, density,vertexData);
            SubDivide(v5, v4, v3, density,vertexData);
            SubDivide(v5, v1, v4, density,vertexData);

            return { vertexData };
        }
        static void SubDivide(glm::vec3 &a, glm::vec3 &b, glm::vec3 &c, int recursions, std::vector<FLOOF::MeshVertex> &vertexData) {
            if (recursions > 0) {
                glm::vec3 v1 = glm::normalize(a + b);
                glm::vec3 v2 = glm::normalize(a + c);
                glm::vec3 v3 = glm::normalize(c + b);

                SubDivide(a, v1, v2, recursions - 1, vertexData);
                SubDivide(c, v2, v3, recursions - 1, vertexData);
                SubDivide(b, v3, v1, recursions - 1, vertexData);
                SubDivide(v3, v2, v1, recursions - 1, vertexData);
            } else {

                PushVerts(a,b,c,vertexData);
            }
        }

        static void PushVerts(glm::vec3 a, glm::vec3 b, glm::vec3 c, std::vector<FLOOF::MeshVertex> &vertexData) {
            glm::vec3 normal = glm::cross(b - a, c - a);
            normal = glm::normalize(normal);
            FLOOF::MeshVertex v{};

            v.Pos = a;
            v.Normal = normal;
            vertexData.push_back(v);

            FLOOF::MeshVertex vertB{};
            v.Pos = b;
            v.Normal = normal;
            vertexData.push_back(v);

            FLOOF::MeshVertex vertC{};
            v.Pos = c;
            v.Normal = normal;
            vertexData.push_back(v);
        }

        glm::vec3 CalcBarycentric(glm::vec3 position, const Triangle& triangle) {

            glm::vec2 pos{ position.x, position.z };
    
            glm::vec2 p1 = glm::vec2(triangle.A.x, triangle.A.z);
            glm::vec2 p2 = glm::vec2(triangle.B.x, triangle.B.z);
            glm::vec2 p3 = glm::vec2(triangle.C.x, triangle.C.z);

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
        float sign (glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
            {
                return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
            }
        bool isInside(const glm::vec3 &position, const Triangle &triangle) {
        //https://stackoverflow.com/questions/2049582/how-to-determine-if-a-point-is-in-a-2d-triangle
                float d1, d2, d3;
                bool has_neg, has_pos;

                d1 = sign(position, triangle.A, triangle.B);
                d2 = sign(position, triangle.B, triangle.C);
                d3 = sign(position, triangle.C, triangle.A);

                has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
                has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

                return !(has_neg && has_pos);
        }
    }
}

