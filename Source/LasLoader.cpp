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
    if (max != glm::vec3(0.f)){
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
    offset = min;
    //offset.y = middle.y;
    // Update min/max
    min -= offset;
    max -= offset;
}

void LasLoader::UpdatePoints() {

    //FindMinMax();
    for (auto& vertex : PointData) {
        if (middle != glm::vec3(0.f))
            vertex.Pos -= offset;
        vertex.Pos *= scale;
    }
    min *= scale;
    max *= scale;
}

void LasLoader::Triangulate() {

    // Squares just use vec2, not vec3, so y = z
    std::vector<Square> squares;

    // width and height
	xSquares = (max.x - min.x)/resolution;
	zSquares = (max.z - min.z)/resolution;

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

    glm::vec2 startPos = squares[0].pos;

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

    startSquare = glm::vec2(squares[0].pos);

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

        // Offset so first vertex starts at  zero
        float off{ resolution / 2.f };
        tempVertex.Pos = glm::vec3(currentSquare.pos.x - off, currentSquare.averageHeight - max.y, currentSquare.pos.y-off);
        //tempVertex.Pos = glm::vec3(currentSquare.pos, currentSquare.averageHeight);

        VertexData.push_back(tempVertex);
    }

    // Create Index
    for (int z = 0; z < zSquares - 1; ++z) {
        for (int x = 0; x < xSquares - 1; ++x) {

            IndexData.emplace_back(x + (xSquares * z));
        	IndexData.emplace_back(x + 1 + (xSquares * (z + 1)));
            IndexData.emplace_back(x + 1 + (xSquares * z));

            IndexData.emplace_back(x + (xSquares * z));
            IndexData.emplace_back(x + (xSquares * (z + 1)));
            IndexData.emplace_back(x + 1 + (xSquares * (z + 1)));

        }
    }

    int xmin = 0, xmax = xSquares, zmin = 0, zmax = zSquares; // The size to draw

    for (int x = 0; x < xmax-1; ++x)
    {
        int z = 0;
        glm::vec3 normal(0.f, 1.f, 0.f);
        VertexData[x].Normal = normal;
    }

    for (int z = 0; z < zmax - 1; ++z)
    {
        int x = 0;
        glm::vec3 normal(0.f, 1.f, 0.f);
        VertexData[z * zmax].Normal = normal;
    }
    xmin++;
    zmin++;

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
            normal = glm::normalize(-normal);

            VertexData[x + (zmax * z)].Normal = normal;
        }

    }
}

std::pair<std::vector<FLOOF::MeshVertex>, std::vector<uint32_t>> LasLoader::GetIndexedData() {
    return {LasLoader::VertexData, LasLoader::IndexData};
}

std::vector<FLOOF::MeshVertex> LasLoader::GetVertexData()
{
    if (!TriangulatedVertexData.empty())
        return TriangulatedVertexData;

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
    TriangulatedVertexData = verts;

    return TriangulatedVertexData;
}

std::vector<std::vector<std::pair<FLOOF::Triangle, FLOOF::Triangle>>> LasLoader::GetTerrainData() {

    int width = (max.x - min.x) / resolution;
    int height = (max.z - min.z) / resolution;

    std::vector<std::vector<std::pair<FLOOF::Triangle, FLOOF::Triangle>>> out(height,
        std::vector<std::pair<FLOOF::Triangle, FLOOF::Triangle>>(width));

    for (int z = 0; z < height - 1; z++) {
        for (int x = 0; x < width - 1; x++) {
            FLOOF::Triangle bottom;
            FLOOF::Triangle top;

            int aIndex = (x + (z * width));
            int bIndex = ((x + 1) + (z * width));
            int cIndex = ((x + 1) + ((z + 1) * width));
            int dIndex = (x + ((z + 1) * width));

            bottom.A = VertexData[aIndex].Pos;
            bottom.B = VertexData[cIndex].Pos;
            bottom.C = VertexData[bIndex].Pos;

            top.A = VertexData[aIndex].Pos;
            top.B = VertexData[dIndex].Pos;
            top.C = VertexData[cIndex].Pos;

            bottom.N = glm::normalize(glm::cross(bottom.B - bottom.A, bottom.C - bottom.A));
            top.N = glm::normalize(glm::cross(top.B - top.A, top.C - top.A));

            out[z][x] = std::make_pair(bottom, top);
        }
    }

    return out;
}




