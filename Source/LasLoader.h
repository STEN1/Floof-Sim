#pragma once
#include <string>
#include <vector>
#include "Floof.h"
#include "Vertex.h"
#include "Physics.h"
class LasLoader {

public:
    LasLoader(const std::string& path);
    std::vector<FLOOF::ColorVertex> GetPointData();
    std::pair<std::vector<FLOOF::MeshVertex>, std::vector<uint32_t>> GetIndexedData();
    std::vector<FLOOF::MeshVertex> GetVertexData();
    std::vector<std::vector<std::pair<FLOOF::Triangle, FLOOF::Triangle>>> GetTerrainData();

private:
    std::vector<FLOOF::ColorVertex> PointData;
    std::vector<FLOOF::MeshVertex> VertexData;
    std::vector<uint32_t> IndexData;
    std::vector<FLOOF::MeshVertex> TriangulatedVertexData;
    std::vector<FLOOF::Triangle> triangles;

    void CalcCenter();
    void FindMinMax();
    void UpdatePoints();
    void Triangulate();


    glm::vec3 min{0.f};
    glm::vec3 max{0.f};
    glm::vec3 middle{0.f};
    glm::vec3 offset{ 0.f };
    float scale{1.f};

    int xSquares{ 0 };
    int zSquares{ 0 };
    float resolution{1.f};
    glm::vec2 startSquare{ 0.f };

};

struct Square {
    glm::vec2 min;
    glm::vec2 max;
    glm::vec2 pos; //center

    float averageHeight{0};
    std::vector<FLOOF::ColorVertex> vertexes;
};

struct Triangle {
    int index[3];
    int neighbor[3];
};