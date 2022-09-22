#pragma once
#include <string>
#include <vector>
#include "Floof.h"
#include "Vertex.h"
class LasLoader {

public:
    LasLoader(const std::string& path);
    std::vector<FLOOF::ColorVertex> GetVertexData();
private:
    std::vector<FLOOF::ColorVertex> VertexData;
    glm::vec3 offset{};
    void Downscale(float multiplier);

    void OffsetCenter();
    void FindMinMax();
    glm::vec3 min{0.f};
    glm::vec3 max{0.f};


};