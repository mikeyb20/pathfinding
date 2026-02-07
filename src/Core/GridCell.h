#pragma once

#include "Core/Types.h"

struct GridCell {
    int x = 0;
    int y = 0;
    float movementCost = 1.0f;
    TerrainType terrain = TerrainType::Open;
    bool walkable = true;
};
