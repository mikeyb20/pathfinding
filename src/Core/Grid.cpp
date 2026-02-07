#include "Core/Grid.h"

Grid::Grid(int width, int height)
    : width_(width), height_(height), cells_(width * height)
{
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            auto& cell = cells_[index(x, y)];
            cell.x = x;
            cell.y = y;
            cell.movementCost = 1.0f;
            cell.terrain = TerrainType::Open;
            cell.walkable = true;
        }
    }
}

bool Grid::inBounds(int x, int y) const {
    return x >= 0 && x < width_ && y >= 0 && y < height_;
}

bool Grid::isWalkable(int x, int y) const {
    return inBounds(x, y) && cells_[index(x, y)].walkable;
}

const GridCell& Grid::getCell(int x, int y) const {
    return cells_[index(x, y)];
}

GridCell& Grid::getCell(int x, int y) {
    return cells_[index(x, y)];
}

void Grid::setTerrain(int x, int y, TerrainType terrain) {
    if (!inBounds(x, y)) return;
    auto& cell = cells_[index(x, y)];
    cell.terrain = terrain;
    cell.movementCost = terrainCost(terrain);
    cell.walkable = (terrain != TerrainType::Wall);
}

void Grid::clear() {
    for (auto& cell : cells_) {
        cell.terrain = TerrainType::Open;
        cell.movementCost = 1.0f;
        cell.walkable = true;
    }
}

std::vector<Vec2i> Grid::getNeighbors4(int x, int y) const {
    std::vector<Vec2i> neighbors;
    neighbors.reserve(4);

    static const int dx[] = {0, 1, 0, -1};
    static const int dy[] = {-1, 0, 1, 0};

    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (isWalkable(nx, ny)) {
            neighbors.push_back({nx, ny});
        }
    }
    return neighbors;
}

std::vector<Vec2i> Grid::getNeighbors8(int x, int y) const {
    std::vector<Vec2i> neighbors;
    neighbors.reserve(8);

    // Cardinal directions first
    static const int dx[] = {0, 1, 0, -1, 1, 1, -1, -1};
    static const int dy[] = {-1, 0, 1, 0, -1, 1, 1, -1};

    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (isWalkable(nx, ny)) {
            neighbors.push_back({nx, ny});
        }
    }

    // Diagonal directions with corner-cutting check
    // Diagonal (1,−1) requires (1,0) and (0,−1) to be walkable
    // Diagonal (1,1) requires (1,0) and (0,1) to be walkable
    // Diagonal (−1,1) requires (−1,0) and (0,1) to be walkable
    // Diagonal (−1,−1) requires (−1,0) and (0,−1) to be walkable
    for (int i = 4; i < 8; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (!isWalkable(nx, ny)) continue;

        // Check that both cardinal neighbors are walkable (no corner cutting)
        bool cardX = isWalkable(x + dx[i], y);
        bool cardY = isWalkable(x, y + dy[i]);
        if (cardX && cardY) {
            neighbors.push_back({nx, ny});
        }
    }
    return neighbors;
}
