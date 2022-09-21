#include "LasLoader.h"
#include <fstream>
#include <sstream>

LasLoader::LasLoader(const std::string &path) : VertexData{} {
    std::ifstream file(path);

    if (!file.is_open()){
        std::cout << "Cant open file: " << path << std::endl;
        return;
    }

    uint32_t vertexCount{};
    file >> vertexCount;
    std::string s;
    std::getline(file, s);
    VertexData.resize(vertexCount);
    for (FLOOF::MeshVertex& vertex : VertexData) {
        std::string line;
        std::getline(file, line);
        std::stringstream ss(line);
        ss >> vertex.Pos.x;
        ss >> vertex.Pos.y;
        ss >> vertex.Pos.z;
    }



}
