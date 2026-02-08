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

nlohmann::json Grid::toJson() const {
    nlohmann::json j;
    j["width"] = width_;
    j["height"] = height_;

    nlohmann::json terrain = nlohmann::json::array();
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            terrain.push_back(terrainToString(cells_[index(x, y)].terrain));
        }
    }
    j["terrain"] = std::move(terrain);
    return j;
}

void Grid::fromJson(const nlohmann::json& j) {
    int w = j.at("width").get<int>();
    int h = j.at("height").get<int>();

    // Resize if dimensions changed
    if (w != width_ || h != height_) {
        width_ = w;
        height_ = h;
        cells_.resize(w * h);
        for (int cy = 0; cy < height_; ++cy) {
            for (int cx = 0; cx < width_; ++cx) {
                auto& cell = cells_[index(cx, cy)];
                cell.x = cx;
                cell.y = cy;
            }
        }
    }

    clear();

    const auto& terrain = j.at("terrain");
    for (int cy = 0; cy < height_; ++cy) {
        for (int cx = 0; cx < width_; ++cx) {
            int idx = cy * width_ + cx;
            if (idx < static_cast<int>(terrain.size())) {
                setTerrain(cx, cy, terrainFromString(terrain[idx].get<std::string>()));
            }
        }
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
