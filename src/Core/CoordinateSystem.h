#pragma once

#include "Core/Types.h"

class CoordinateSystem {
public:
    CoordinateSystem(float cellSize, Vec2f offset);

    void setCellSize(float size) { cellSize_ = size; }
    float getCellSize() const { return cellSize_; }

    void setOffset(Vec2f offset) { offset_ = offset; }
    Vec2f getOffset() const { return offset_; }

    // Convert grid coordinates to screen pixel position (top-left of cell)
    Vec2f gridToScreen(int gx, int gy) const;
    Vec2f gridToScreen(Vec2i grid) const;

    // Convert screen pixel position to grid coordinates
    Vec2i screenToGrid(float sx, float sy) const;
    Vec2i screenToGrid(Vec2f screen) const;

private:
    float cellSize_;
    Vec2f offset_;
};
