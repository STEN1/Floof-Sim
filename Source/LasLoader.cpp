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

}
