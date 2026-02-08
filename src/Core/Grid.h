#pragma once

#include <vector>
#include <nlohmann/json.hpp>
#include "Core/Types.h"
#include "Core/GridCell.h"

class Grid {
public:
    Grid(int width, int height);

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    bool inBounds(int x, int y) const;
    bool isWalkable(int x, int y) const;

    const GridCell& getCell(int x, int y) const;
    GridCell& getCell(int x, int y);

    void setTerrain(int x, int y, TerrainType terrain);
    void clear();

    nlohmann::json toJson() const;
    void fromJson(const nlohmann::json& j);

    // Returns walkable neighbors (4-directional)
    std::vector<Vec2i> getNeighbors4(int x, int y) const;
    // Returns walkable neighbors (8-directional with corner-cutting check)
    std::vector<Vec2i> getNeighbors8(int x, int y) const;

private:
    int width_;
    int height_;
    std::vector<GridCell> cells_;

    int index(int x, int y) const { return y * width_ + x; }
};
