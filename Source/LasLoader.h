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
    std::vector<std::vector<std::pair<FLOOF::Triangle, FLOOF::Triangle>>> GetTerrainData();

    float GetMinY(){return -max.y;}
private:
    std::vector<FLOOF::ColorVertex> PointData;
    std::vector<FLOOF::MeshVertex> VertexData;
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

struct height {
    int count{0};
    float sum{0.f};
};

struct lasHeader {
    char fileSignature[4];
    unsigned short sourceID;
    unsigned short globalEncoding;
    unsigned long GUID1;
    unsigned short GUID2;
    unsigned short GUID3;
    unsigned char GUID4[8];
    unsigned char versionMajor;
    unsigned char versionMinor;
    char systemIdentifier[32];
    char generatingSoftware[32];
    unsigned short creationDay;
    unsigned short creationYear;
    unsigned short headerSize;
    unsigned long offsetToPointData;
    unsigned long numberVariableLengthRecords;
    unsigned char pointDataRecordFormat; // Format
    unsigned short pointDataRecordLength;
    unsigned long legacyNumberPointsRecords;
    unsigned long legacyNumberPointReturn[5];
    double xScaleFactor;
    double yScaleFactor;
    double zScaleFactor;
    double xOffset;
    double yOffset;
    double zOffset;
    double maxX;
    double minX;
    double maxY;
    double minY;
    double maxZ;
    double minZ;
    unsigned long long startWaveformDataPacketRecord;
    unsigned long long startFirstExtendedVariableLengthRecord;
    unsigned long numberExtendedVariableLengthRecords;
    unsigned long long numberPointRecords;
    unsigned long long numberPointsByReturn;
};

struct lasVariableLengthRecords{
    // Variable Length Records
    unsigned short lasReserved;
    char UserID[16];
    unsigned short recordID;
    unsigned short recordLengthAfterHeader;
    char lasDescription[32];
};

struct lasPointData1{
    // Point Data
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

struct lasPointData2{
    // Point Data
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

