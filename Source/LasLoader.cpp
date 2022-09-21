#include "LasLoader.h"
#include <fstream>
#include <sstream>

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
        ss >> tempVertex.Pos.y;
        ss >> tempVertex.Pos.z;

        tempVertex.Color = glm::vec3(1.f,1.f,1.f);

        VertexData.push_back(tempVertex);
    }

    Downscale();

}

std::vector<FLOOF::ColorVertex> LasLoader::GetVertexData() {
    return VertexData;
}

void LasLoader::Downscale() {

    float multiplier{0.001};
    glm::vec3 offset(-876831.f,-2260896.f,-348.f);
    for (auto& vertex : VertexData) {
        vertex.Pos = (vertex.Pos + offset) * multiplier;
    }
}
