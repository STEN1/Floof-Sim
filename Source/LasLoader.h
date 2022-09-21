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
    void FindMinMax();
    glm::vec3 min{};
    glm::vec3 max{};



};