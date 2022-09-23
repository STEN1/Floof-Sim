#pragma once
#include <string>
#include <vector>
#include "Floof.h"
#include "Vertex.h"
class LasLoader {

public:
    LasLoader(const std::string& path);
    std::vector<FLOOF::ColorVertex> GetPointData();
    std::pair<std::vector<FLOOF::MeshVertex>, std::vector<uint32_t>> GetIndexedData();

private:
    std::vector<FLOOF::ColorVertex> PointData;
    std::vector<FLOOF::MeshVertex> VertexData;
    std::vector<uint32_t> IndexData;
    glm::vec3 offset{};

    void CalcCenter();
    void FindMinMax();
    void UpdatePoints();
    glm::vec3 min{0.f};
    glm::vec3 max{0.f};
    glm::vec3 middle{0.f};
    float scale{1.f};

    float resolution{5.f};


    void Triangulate();



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