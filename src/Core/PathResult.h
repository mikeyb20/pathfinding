#pragma once

#include <vector>
#include "Core/Types.h"

struct PathResult {
    std::vector<Vec2i> path;
    float totalCost = 0.0f;
    int nodesExpanded = 0;
    float computeTimeMs = 0.0f;
    std::vector<Vec2i> visitedOrder;

    bool found() const { return !path.empty(); }
};
