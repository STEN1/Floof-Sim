#include "LasLoader.h"
#include <fstream>
#include <sstream>
#include <limits>
LasLoader::LasLoader(const std::string &path) : PointData{} {
    std::string txt (".txt");
    std::string lasbin (".lasbin");

    if (path.find(txt) != std::string::npos)
        ReadTxt(path);

    else if (path.find(lasbin) != std::string::npos)
        ReadBin(path);

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

	// Update min/max
    min -= offset;
    max -= offset;
}

void LasLoader::UpdatePoints() {

    for (auto& vertex : PointData) {
        if (middle != glm::vec3(0.f))
            vertex.Pos -= offset;
        vertex.Pos *= scale;
    }
    min *= scale;
    max *= scale;
}

void LasLoader::Triangulate() {

    // width and height
    xSquares = (max.x - min.x);
    zSquares = (max.z - min.z);

    // Save all height data for each vertex
    std::vector<std::vector<std::vector<float>>> heightmap(zSquares, std::vector<std::vector<float>>(xSquares));
    for (auto &vertex: PointData) {
        int xPos = vertex.Pos.x;
        int zPos = vertex.Pos.z;

        if (xPos < 0.f || xPos > xSquares-1
            || zPos < 0.f || zPos > zSquares-1) {
            continue;
        }
        heightmap[zPos][xPos].push_back(vertex.Pos.y);
    }

    std::vector<std::pair<int, int>> noHeight;

    // Calculate average height for each vertex and push
    for (int z = 0; z < zSquares; ++z) {
        for (int x = 0; x < xSquares; ++x) {
            float average{ 0.f };
            int count{ 0 };
            for (auto& height : heightmap[z][x]) {
                average += height;
                count++;
            }
            float y;
            if (count == 0)
            {
                y = -max.y;
                noHeight.push_back(std::make_pair(x,z));
            }
                
            else
            	y = (average / count) - max.y;
            FLOOF::MeshVertex temp{};
            temp.Pos = glm::vec3(x, y, z);
            VertexData.push_back(temp);
        }
    }

    // Calculate average height if no height
    for (auto& vertex : noHeight)
    {
        float averageHeight{ 0.f };
        int x = vertex.first;
        int z = vertex.second;
        if (x <= 0 || x >= xSquares-1 || z <= 0 || z >= zSquares-1)
        {
            continue;
        }
        else
        {
            averageHeight += VertexData[(x - 1) + (z * xSquares)].Pos.y;
            averageHeight += VertexData[(x - 1) + ((z - 1) * xSquares)].Pos.y;
            averageHeight += VertexData[x + ((z - 1) * xSquares)].Pos.y;
            averageHeight += VertexData[(x + 1) + ((z - 1) * xSquares)].Pos.y;
            averageHeight += VertexData[(x + 1) + (z * xSquares)].Pos.y;
            averageHeight += VertexData[(x + 1) + ((z + 1) * xSquares)].Pos.y;
            averageHeight += VertexData[x + ((z + 1) * xSquares)].Pos.y;
            averageHeight += VertexData[(x - 1) + ((z + 1) * xSquares)].Pos.y;

            VertexData[x + (z * xSquares)].Pos.y = averageHeight / 8;
        }
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

    // Calculate smooth normals
    int xmin = 0, xmax = xSquares, zmin = 0, zmax = zSquares; // The size to draw

    // Normals for rest
    for (int z = zmin; z < zmax; z++) {
        for (int x = xmin; x < xmax; x++) {
	        if (z == zmin || z == zmax-1 || x == xmin || x == xmax-1)
	        {
                VertexData[x + (xmax * z)].Normal = glm::vec3(0.f, 1.f, 0.f);
	        }
            else
            {
                glm::vec3 a(VertexData[x + (xmax * z)].Pos);
                glm::vec3 b(VertexData[x + 1 + (xmax * z)].Pos);
                glm::vec3 c(VertexData[x + 1 + (xmax * (z + 1))].Pos);
                glm::vec3 d(VertexData[x + (xmax * (z + 1))].Pos);
                glm::vec3 e(VertexData[x - 1 + (xmax * z)].Pos);
                glm::vec3 f(VertexData[x - 1 + (xmax * (z - 1))].Pos);
                glm::vec3 g(VertexData[x + (xmax * (z - 1))].Pos);

                auto n0 = glm::cross(b - a, c - a);
                auto n1 = glm::cross(c - a, d - a);
                auto n2 = glm::cross(d - a, e - a);
                auto n3 = glm::cross(e - a, f - a);
                auto n4 = glm::cross(f - a, g - a);
                auto n5 = glm::cross(g - a, b - a);

                glm::vec3 normal = n0 + n1 + n2 + n3 + n4 + n5;
                normal = glm::normalize(-normal);

                VertexData[x + (xmax * z)].Normal = normal;
            }
        }
    }
}


std::pair<std::vector<FLOOF::MeshVertex>, std::vector<uint32_t>> LasLoader::GetIndexedData() {
    return {LasLoader::VertexData, LasLoader::IndexData};
}

std::vector<FLOOF::MeshVertex> LasLoader::GetVertexData()
{
    std::vector<FLOOF::MeshVertex> out;
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

        out.push_back(tempA);
        out.push_back(tempB);
        out.push_back(tempC);
    }
    return out;
}

std::vector<std::vector<std::pair<FLOOF::Triangle, FLOOF::Triangle>>> LasLoader::GetTerrainData() {

    int width = (max.x - min.x);
    int height = (max.z - min.z);

    std::vector<std::vector<std::pair<FLOOF::Triangle, FLOOF::Triangle>>> out(height - 1,
        std::vector<std::pair<FLOOF::Triangle, FLOOF::Triangle>>(width - 1));

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

void LasLoader::ReadBin(const std::string &path) {

    std::ifstream is(path, std::ios::binary | std::ios::ate);
    auto size = is.tellg();
    std::vector<glm::vec3> lasDataPoints(size / sizeof(glm::vec3));
    is.seekg(0);
    is.read(reinterpret_cast<char *>(lasDataPoints.data()), size);

    for (auto& point : lasDataPoints) {
        FLOOF::ColorVertex tempVertex{};
        tempVertex.Pos.x = point.x;
        tempVertex.Pos.y = point.z;
        tempVertex.Pos.z = point.y;
        tempVertex.Color = glm::vec3(1.f,1.f,1.f);
        PointData.push_back(tempVertex);
    }

}

void LasLoader::ReadTxt(const std::string &path) {
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

}
