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

    //scale = 0.01f;
    CalcCenter();
    UpdatePoints();

    Triangulate();
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
        vertex.Pos *= scale;
    }
    min *= scale;
    max *= scale;
}

void LasLoader::Triangulate() {

    std::vector<Square> squares;

    // Create right number of squares
    for (float z = min.z; z < max.z; z += resolution) {
        for (float x = min.x; x < max.y; x += resolution){
            Square tempSquare;
            tempSquare.min = glm::vec2(x,z);
            tempSquare.max = glm::vec2(x+resolution,z+resolution);
            tempSquare.mid = tempSquare.max/tempSquare.min;
            squares.push_back(tempSquare);
        }
    }

    // Put points in correct squares
    for (auto& vertex : VertexData){
        for (auto& currentSquare : squares) {
            if (vertex.Pos.x >= currentSquare.min.x
            && vertex.Pos.z >= currentSquare.min.y
            && vertex.Pos.x < currentSquare.max.x
            && vertex.Pos.z < currentSquare.max.y)
            {
                currentSquare.vertexes.push_back(vertex);
            }
        }
    }

    // Find the average height of each square
    for (auto& currentSquare : squares) {
        currentSquare.averageHeight = 0.f;
        for (auto& vertex : currentSquare.vertexes) {
            currentSquare.averageHeight += vertex.Pos.y;
        }
        currentSquare.averageHeight /= currentSquare.vertexes.size();
    }

    

}




