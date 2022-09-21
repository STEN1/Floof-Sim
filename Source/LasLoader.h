#pragma once
#include <string>
#include <vector>
#include "Floof.h"
#include "Vertex.h"
class LasLoader {

public:
    LasLoader(const std::string& path);
    std::vector<FLOOF::ColorVertex> GetVertexData();



private:
    std::vector<FLOOF::ColorVertex> VertexData;

    void Downscale();



};