#pragma once
#include <string>
#include <vector>
#include "Floof.h"
#include "Vertex.h"
class LasLoader {

public:
    LasLoader(const std::string& path);

private:
    std::vector<FLOOF::MeshVertex> VertexData;


};