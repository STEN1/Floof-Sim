#include "LasLoader.h"
#include <fstream>
#include <sstream>
#include <limits>
LasLoader::LasLoader(const std::string &path) : VertexData{} {
    std::ifstream file(path);

    if (!file.is_open()){
        std::cout << "Cant open file: " << path << std::endl;
        return;
    }

    std::string line;
    FLOOF::ColorVertex tempVertex{};

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        ss >> tempVertex.Pos.x;
        ss >> tempVertex.Pos.z;
        ss >> tempVertex.Pos.y;
        tempVertex.Color = glm::vec3(1.f,1.f,1.f);
        VertexData.push_back(tempVertex);
    }

    m_Scale = 0.01f;
    CalcCenter();
    UpdatePoints();

}

std::vector<FLOOF::ColorVertex> LasLoader::GetVertexData() {
    return VertexData;
}


void LasLoader::FindMinMax() {

    // Check if min/max already found
    if (min != glm::vec3(0.f) && max != glm::vec3(0.f)){
        return;
    }
    min = glm::vec3(std::numeric_limits<float>::max());
    max = glm::vec3(std::numeric_limits<float>::min());

    for (auto& vertex : VertexData) {
        if (min.x > vertex.Pos.x) {min.x = vertex.Pos.x;}
        else if (max.x < vertex.Pos.x) {max.x = vertex.Pos.x;}
        if (min.z > vertex.Pos.z) {min.z = vertex.Pos.z;}
        else if (max.z < vertex.Pos.z) {max.z = vertex.Pos.z;}
        if (min.y > vertex.Pos.y) {min.y = vertex.Pos.y;}
        else if (max.y < vertex.Pos.y) {max.y = vertex.Pos.y;}
    }
}

void LasLoader::CalcCenter() {

    FindMinMax();

    // Find middle
    middle = min + (max-min)/2.f;

    // Update min/max
    min -= middle;
    max -= middle;
}

void LasLoader::UpdatePoints() {

    FindMinMax();
    for (auto& vertex : VertexData) {
        if (middle != glm::vec3(0.f))
            vertex.Pos -= middle;
        vertex.Pos *= m_Scale;
    }
    min *= m_Scale;
    max *= m_Scale;
}
