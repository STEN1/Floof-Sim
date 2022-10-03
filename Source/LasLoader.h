#pragma once
#include <string>
#include <vector>
#include "Floof.h"
#include "Vertex.h"
#include "Physics.h"
class LasLoader {

public:
    LasLoader(const std::string& path);
    std::vector<FLOOF::ColorVertex> GetPointData();
    std::pair<std::vector<FLOOF::MeshVertex>, std::vector<uint32_t>> GetIndexedData();
    std::vector<FLOOF::MeshVertex> GetVertexData();
    std::pair<std::vector<FLOOF::ColorNormalVertex>, std::vector<uint32_t>> GetIndexedColorNormalVertexData();
    std::vector<std::vector<std::pair<FLOOF::Triangle, FLOOF::Triangle>>> GetTerrainData();

    float GetMinY(){return -max.y;}
private:
    std::vector<FLOOF::ColorVertex> PointData;
    std::vector<FLOOF::MeshVertex> VertexData;
    std::vector<FLOOF::ColorNormalVertex> ColorNormalVertexData;
    std::vector<uint32_t> IndexData;
    std::vector<FLOOF::MeshVertex> TriangulatedVertexData;
    std::vector<FLOOF::Triangle> triangles;

    void ReadTxt(const std::string& path);
    void ReadBin(const std::string& path);
    void ReadLas(const std::string& path);


    void CalcCenter();
    void FindMinMax();
    void UpdatePoints();
    void Triangulate();


    glm::vec3 min{0.f};
    glm::vec3 max{0.f};
    glm::vec3 middle{0.f};
    glm::vec3 offset{ 0.f };
    float scale{1.f};

    int xSquares{ 0 };
    int zSquares{ 0 };
};

struct HeightAndColor {
    int count{0};
    float sum{0.f};
    glm::vec3 color{ 1.f };
};

struct lasHeader {

    char fileSignature[4];
    uint16_t sourceID;
    uint16_t globalEncoding;
    uint32_t GUID1;
    uint16_t GUID2;
    uint16_t GUID3;
    uint8_t GUID4[8];
    uint8_t versionMajor;
    uint8_t versionMinor;
    char systemIdentifier[32];
    char generatingSoftware[32];
    uint16_t creationDay;
    uint16_t creationYear;
    uint16_t headerSize;
    uint32_t offsetToPointData; // The actual number of bytes from the beginning of the file to the first field of the first point record.
    uint32_t numberVariableLengthRecords; // Length of VLR
    uint8_t pointDataRecordFormat; // Format
    uint16_t pointDataRecordLength; // The size, in bytes, of the Point Data Record
    int32_t legacyNumberPointsRecords;
    int32_t legacyNumberPointReturn[5];
    double xScaleFactor, yScaleFactor, zScaleFactor;
    double xOffset, yOffset, zOffset;
    double maxX, minX;
    double maxY, minY;
    double maxZ, minZ;
};

struct lasVariableLengthRecords{
    // Variable Length Records
    unsigned short lasReserved;
    char UserID[16];
    unsigned short recordID;
    unsigned short recordLengthAfterHeader;
    char lasDescription[32];
};

struct lasPointData1 {
    long xPos;
    long yPos;
    long zPos;
    unsigned short intensity;
    signed char flags;
    unsigned char classificaton;
    signed char scanAngle;
    unsigned char userData;
    unsigned short pointSourceID;
    double GPSTime;
};

// Can't use struct directly because of padding of the size of the struct
struct lasPointData2 {
    long xPos;
    long yPos;
    long zPos;
    unsigned short intensity;
    signed char flags;
    unsigned char classificaton;
    signed char scanAngle;
    unsigned char userData;
    unsigned short pointSourceID;
    unsigned short red;
    unsigned short green;
    unsigned short blue;


};


