#pragma once

#include <vector>
#include <functional>
#include "Core/Types.h"

struct SearchState {
    std::vector<Vec2i> visitedOrder;
    std::vector<Vec2i> frontier;
    std::vector<Vec2i> currentPath;
    bool finished = false;
    bool pathFound = false;
};

struct Vec2iHash {
    std::size_t operator()(const Vec2i& v) const {
        auto h1 = std::hash<int>{}(v.x);
        auto h2 = std::hash<int>{}(v.y);
        return h1 ^ (h2 << 16);
    }
};
