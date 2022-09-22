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

    void CalcCenter();
    void FindMinMax();
    void UpdatePoints();
    glm::vec3 min{0.f};
    glm::vec3 max{0.f};
    glm::vec3 middle{0.f};
    float m_Scale{1.f};

};