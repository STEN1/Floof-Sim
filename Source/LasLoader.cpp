#include "LasLoader.h"
#include <fstream>
#include <sstream>
#include <set>
#include <limits>
LasLoader::LasLoader(const std::string &path) : VertexData{} {
    std::ifstream file(path);

    if (!file.is_open()){
        std::cout << "Cant open file: " << path << std::endl;
        return;
    }

    std::string line;
    FLOOF::ColorVertex tempVertex{};

    std::multiset<float> xValues;
    std::multiset<float> zValues;
    std::multiset<float> yValues;

    {
        std::getline(file, line);
        std::stringstream ss(line);
        ss >> offset.x;
        ss >> offset.z;
        ss >> offset.y;
        file.clear();
        file.seekg (0);
    }

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        ss >> tempVertex.Pos.x;
        ss >> tempVertex.Pos.z;
        ss >> tempVertex.Pos.y;

        tempVertex.Pos -= offset;

        tempVertex.Color = glm::vec3(1.f,1.f,1.f);

        VertexData.push_back(tempVertex);
    }

    Downscale(0.01);



}

std::vector<FLOOF::ColorVertex> LasLoader::GetVertexData() {
    return VertexData;
}

void LasLoader::Downscale(float multiplier) {
    for (auto& vertex : VertexData) {
        vertex.Pos *= multiplier;
    }
}

void LasLoader::FindMinMax() {

    min = glm::vec3(std::numeric_limits<float>::max());
    max = glm::vec3(std::numeric_limits<float>::min());

    for (auto& vertex : VertexData) {
        if (min.x < vertex.Pos.x) {min.x = vertex.Pos.x;}
        else if (max.x > vertex.Pos.x) {max.x = vertex.Pos.x;}
        if (min.z < vertex.Pos.z) {min.z = vertex.Pos.z;}
        else if (max.z > vertex.Pos.z) {max.z = vertex.Pos.z;}
        if (min.y < vertex.Pos.y) {min.y = vertex.Pos.y;}
        else if (max.y > vertex.Pos.y) {max.y = vertex.Pos.y;}
    }
}
