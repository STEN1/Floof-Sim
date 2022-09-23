#include "LasLoader.h"
#include <fstream>
#include <sstream>
#include <limits>
LasLoader::LasLoader(const std::string &path) : PointData{} {
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
        PointData.push_back(tempVertex);
    }

    //scale = 0.01f;
    CalcCenter();
    UpdatePoints();

    Triangulate();
}

std::vector<FLOOF::ColorVertex> LasLoader::GetVertexData() {
    return PointData;
}


void LasLoader::FindMinMax() {

    // Check if min/max already found
    if (min != glm::vec3(0.f) && max != glm::vec3(0.f)){
        return;
    }
    min = glm::vec3(std::numeric_limits<float>::max());
    max = glm::vec3(std::numeric_limits<float>::min());

    for (auto& vertex : PointData) {
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
    for (auto& vertex : PointData) {
        if (middle != glm::vec3(0.f))
            vertex.Pos -= middle;
        vertex.Pos *= scale;
    }
    min *= scale;
    max *= scale;
}

void LasLoader::Triangulate() {

    std::vector<Square> squares;

    int xSquares = (max.x - min.x)/resolution;
    int zSquares = (max.z - min.z)/resolution;

    // Create right number of squares
    for (float z = min.z; z < max.z; z += resolution) {
        for (float x = min.x; x < max.x; x += resolution){
            Square tempSquare;
            tempSquare.min = glm::vec2(x,z);
            tempSquare.max = glm::vec2(x+resolution,z+resolution);
            tempSquare.pos = tempSquare.max/tempSquare.min;
            squares.push_back(tempSquare);
        }
    }

    // Put points in correct squares
    for (auto& vertex : PointData){
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

    // Find the average height of each square and create vertex
    for (auto& currentSquare : squares) {
        currentSquare.averageHeight = 0.f;
        for (auto& vertex : currentSquare.vertexes) {
            currentSquare.averageHeight += vertex.Pos.y;
        }
        currentSquare.averageHeight /= currentSquare.vertexes.size();

        FLOOF::MeshVertex tempVertex{};
        tempVertex.Pos = glm::vec3(currentSquare.pos, currentSquare.averageHeight);
        VertexData.push_back(tempVertex);
    }

    // Create Index
    for (int z = 0; z < zSquares; ++z) {
        for (int x = 0; x < xSquares; ++x) {

            IndexData.emplace_back(x + (zSquares * z));
            IndexData.emplace_back(x + 1 + (zSquares * z));
            IndexData.emplace_back(x + 1 + (zSquares * (z + 1)));

            IndexData.emplace_back(x + (zSquares * z));
            IndexData.emplace_back(x + 1 + (zSquares * (z + 1)));
            IndexData.emplace_back(x + (zSquares * (z + 1)));
        }
    }

    int xmin = 1, xmax = xSquares, ymin = 1, ymax = zSquares; // The size to draw

    for (int y = ymin; y < ymax - 1; y++)
    {
        for (int x = xmin; x < xmax - 1; x++)
        {
            glm::vec3 a(VertexData[x + (ymax * y)].Pos);
            glm::vec3 b(VertexData[x + 1 + (ymax * y)].Pos);
            glm::vec3 c(VertexData[x + 1 + (ymax * (y + 1))].Pos);
            glm::vec3 d(VertexData[x + (ymax * (y + 1))].Pos);
            glm::vec3 e(VertexData[x - 1 + (ymax * y)].Pos);
            glm::vec3 f(VertexData[x - 1 + (ymax * (y - 1))].Pos);
            glm::vec3 g(VertexData[x + (ymax * (y - 1))].Pos);

            auto n0 = glm::cross(b - a, c - a);
            auto n1 = glm::cross(c - a, d - a);
            auto n2 = glm::cross(d - a, e - a);
            auto n3 = glm::cross(e - a, f - a);
            auto n4 = glm::cross(f - a, g - a);
            auto n5 = glm::cross(g - a, b - a);

            glm::vec3 normal = n0 + n1 + n2 + n3 + n4 + n5;
            normal = glm::normalize(normal);

            VertexData[x + (ymax * y)].Normal = normal;
        }

    }
}




