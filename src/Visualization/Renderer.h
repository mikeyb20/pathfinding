#pragma once

#include "Core/Grid.h"
#include "Core/CoordinateSystem.h"
#include "Core/SearchState.h"

class Renderer {
public:
    void drawGrid(const Grid& grid, const CoordinateSystem& coords);
    void drawSearchOverlay(const SearchState& state, const CoordinateSystem& coords);
    void drawStartGoal(Vec2i start, Vec2i goal, const CoordinateSystem& coords);
    void drawPath(const std::vector<Vec2i>& path, const CoordinateSystem& coords);
};
