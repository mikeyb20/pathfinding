#include "Core/CoordinateSystem.h"
#include <cmath>

CoordinateSystem::CoordinateSystem(float cellSize, Vec2f offset)
    : cellSize_(cellSize), offset_(offset)
{
}

Vec2f CoordinateSystem::gridToScreen(int gx, int gy) const {
    return {
        offset_.x + gx * cellSize_,
        offset_.y + gy * cellSize_
    };
}

Vec2f CoordinateSystem::gridToScreen(Vec2i grid) const {
    return gridToScreen(grid.x, grid.y);
}

Vec2i CoordinateSystem::screenToGrid(float sx, float sy) const {
    return {
        static_cast<int>(std::floor((sx - offset_.x) / cellSize_)),
        static_cast<int>(std::floor((sy - offset_.y) / cellSize_))
    };
}

Vec2i CoordinateSystem::screenToGrid(Vec2f screen) const {
    return screenToGrid(screen.x, screen.y);
}
