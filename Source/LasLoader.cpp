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

std::vector<FLOOF::ColorVertex> LasLoader::GetPointData() {
    return LasLoader::PointData;
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

    // Squares just use vec2, not vec3, so y = z
    std::vector<Square> squares;

    int xSquares = (max.x - min.x)/resolution;
    int zSquares = (max.z - min.z)/resolution;

    // Create right number of squares
    for (float z = min.z; z < max.z; z += resolution) {
        for (float x = min.x; x < max.x; x += resolution){
            Square tempSquare;
            tempSquare.min = glm::vec2(x,z);
            tempSquare.max = glm::vec2(x+resolution,z+resolution);
            tempSquare.pos = tempSquare.min + (tempSquare.max - tempSquare.min)/glm::vec2(2.f);
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
            ASSERT(!currentSquare.vertexes.empty());
            // TODO : Calculate average neighbor height if no vertexes
        }
        currentSquare.averageHeight /= currentSquare.vertexes.size();

        FLOOF::MeshVertex tempVertex{};
        tempVertex.Pos = glm::vec3(currentSquare.pos.x, currentSquare.averageHeight, currentSquare.pos.y);
        //tempVertex.Pos = glm::vec3(currentSquare.pos, currentSquare.averageHeight);

        VertexData.push_back(tempVertex);
    }

    // Create Index
    for (int z = 1; z < zSquares - 1; ++z) {
        for (int x = 1; x < xSquares - 1; ++x) {

            IndexData.emplace_back(x + (zSquares * z));
        	IndexData.emplace_back(x + 1 + (zSquares * (z + 1)));
            IndexData.emplace_back(x + 1 + (zSquares * z));

            IndexData.emplace_back(x + (zSquares * z));
            IndexData.emplace_back(x + (zSquares * (z + 1)));
            IndexData.emplace_back(x + 1 + (zSquares * (z + 1)));

        }
    }

    int xmin = 1, xmax = xSquares, zmin = 1, zmax = zSquares; // The size to draw

    for (int z = zmin; z < zmax - 1; z++)
    {
        for (int x = xmin; x < xmax - 1; x++)
        {
            glm::vec3 a(VertexData[x + (zmax * z)].Pos);
            glm::vec3 b(VertexData[x + 1 + (zmax * z)].Pos);
            glm::vec3 c(VertexData[x + 1 + (zmax * (z + 1))].Pos);
            glm::vec3 d(VertexData[x + (zmax * (z + 1))].Pos);
            glm::vec3 e(VertexData[x - 1 + (zmax * z)].Pos);
            glm::vec3 f(VertexData[x - 1 + (zmax * (z - 1))].Pos);
            glm::vec3 g(VertexData[x + (zmax * (z - 1))].Pos);

            auto n0 = glm::cross(b - a, c - a);
            auto n1 = glm::cross(c - a, d - a);
            auto n2 = glm::cross(d - a, e - a);
            auto n3 = glm::cross(e - a, f - a);
            auto n4 = glm::cross(f - a, g - a);
            auto n5 = glm::cross(g - a, b - a);

            glm::vec3 normal = n0 + n1 + n2 + n3 + n4 + n5;
            normal = glm::normalize(normal);

            VertexData[x + (zmax * z)].Normal = normal;
        }

    }
}

std::pair<std::vector<FLOOF::MeshVertex>, std::vector<uint32_t>> LasLoader::GetIndexedData() {
    return {LasLoader::VertexData, LasLoader::IndexData};
}

std::vector<FLOOF::MeshVertex> LasLoader::GetVertexData()
{
    std::vector<FLOOF::MeshVertex> verts;
    int i = 0;
    while (i != IndexData.size()) {
        FLOOF::MeshVertex tempA;
        tempA = VertexData[IndexData[i]];
        i++;

        FLOOF::MeshVertex tempB;
        tempB = VertexData[IndexData[i]];
        i++;

        FLOOF::MeshVertex tempC;
        tempC = VertexData[IndexData[i]];
        i++;

        auto ab = tempB.Pos - tempA.Pos;
        auto ac = tempC.Pos - tempA.Pos;

        glm::vec3 normal = glm::normalize(glm::cross(ab, ac));
        tempA.Normal = normal;
        tempB.Normal = normal;
        tempC.Normal = normal;

        tempA.UV.x = tempA.Pos.x;
        tempA.UV.y = tempA.Pos.y;
        tempB.UV.x = tempB.Pos.x;
        tempB.UV.y = tempB.Pos.y;
        tempC.UV.x = tempC.Pos.x;
        tempC.UV.y = tempC.Pos.y;

        verts.push_back(tempA);
        verts.push_back(tempB);
        verts.push_back(tempC);

    }

    return verts;
}

std::vector<FLOOF::Triangle> LasLoader::GetTriangles() {

    std::vector<FLOOF::Triangle> tris;

    int i = 0;
    while (i != IndexData.size()) {
        glm::vec3 pos = VertexData[IndexData[i]].Pos;
        FLOOF::Triangle tempTri;
        tempTri.A = pos;
        i++;

        pos = VertexData[IndexData[i]].Pos;
        tempTri.B = pos;
        i++;

        pos = VertexData[IndexData[i]].Pos;
        tempTri.C = pos;
        i++;

        auto ab = tempTri.A - tempTri.B;
        auto ac = tempTri.A - tempTri.C;
        tempTri.N = glm::normalize(glm::cross(ab, ac));

        tris.push_back(tempTri);
    }

    return tris;
}




