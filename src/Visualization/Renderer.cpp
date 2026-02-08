#include "Visualization/Renderer.h"
#include "raylib.h"

namespace {

Color terrainColor(TerrainType t) {
    switch (t) {
        case TerrainType::Open:   return {240, 240, 240, 255};
        case TerrainType::Wall:   return {40, 40, 40, 255};
        case TerrainType::Water:  return {70, 130, 200, 255};
        case TerrainType::Mud:    return {160, 120, 60, 255};
        case TerrainType::Forest: return {50, 160, 70, 255};
    }
    return LIGHTGRAY;
}

} // namespace

void Renderer::drawGrid(const Grid& grid, const CoordinateSystem& coords) {
    float cellSize = coords.getCellSize();

    // Draw cells
    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            const auto& cell = grid.getCell(x, y);
            Vec2f pos = coords.gridToScreen(x, y);
            DrawRectangle(
                static_cast<int>(pos.x), static_cast<int>(pos.y),
                static_cast<int>(cellSize), static_cast<int>(cellSize),
                terrainColor(cell.terrain)
            );
        }
    }

    // Draw grid lines
    Vec2f origin = coords.gridToScreen(0, 0);
    float totalW = grid.getWidth() * cellSize;
    float totalH = grid.getHeight() * cellSize;

    for (int x = 0; x <= grid.getWidth(); ++x) {
        float px = origin.x + x * cellSize;
        DrawLineV(
            {px, origin.y},
            {px, origin.y + totalH},
            {200, 200, 200, 255}
        );
    }
    for (int y = 0; y <= grid.getHeight(); ++y) {
        float py = origin.y + y * cellSize;
        DrawLineV(
            {origin.x, py},
            {origin.x + totalW, py},
            {200, 200, 200, 255}
        );
    }
}

void Renderer::drawSearchOverlay(const SearchState& state, const CoordinateSystem& coords) {
    float cellSize = coords.getCellSize();
    float pad = 1.0f;

    // Visited nodes
    for (const auto& v : state.visitedOrder) {
        Vec2f pos = coords.gridToScreen(v);
        DrawRectangle(
            static_cast<int>(pos.x + pad), static_cast<int>(pos.y + pad),
            static_cast<int>(cellSize - 2 * pad), static_cast<int>(cellSize - 2 * pad),
            {173, 216, 230, 160} // light blue
        );
    }

    // Frontier nodes
    for (const auto& f : state.frontier) {
        Vec2f pos = coords.gridToScreen(f);
        DrawRectangle(
            static_cast<int>(pos.x + pad), static_cast<int>(pos.y + pad),
            static_cast<int>(cellSize - 2 * pad), static_cast<int>(cellSize - 2 * pad),
            {144, 238, 144, 180} // light green
        );
    }
}

void Renderer::drawCompSearchOverlay(const SearchState& state, const CoordinateSystem& coords) {
    float cellSize = coords.getCellSize();
    float pad = 1.0f;

    // Visited nodes — light orange
    for (const auto& v : state.visitedOrder) {
        Vec2f pos = coords.gridToScreen(v);
        DrawRectangle(
            static_cast<int>(pos.x + pad), static_cast<int>(pos.y + pad),
            static_cast<int>(cellSize - 2 * pad), static_cast<int>(cellSize - 2 * pad),
            {230, 180, 140, 160}
        );
    }

    // Frontier nodes — light pink
    for (const auto& f : state.frontier) {
        Vec2f pos = coords.gridToScreen(f);
        DrawRectangle(
            static_cast<int>(pos.x + pad), static_cast<int>(pos.y + pad),
            static_cast<int>(cellSize - 2 * pad), static_cast<int>(cellSize - 2 * pad),
            {238, 180, 180, 180}
        );
    }
}

void Renderer::drawCompPath(const std::vector<Vec2i>& path, const CoordinateSystem& coords) {
    float cellSize = coords.getCellSize();
    float pad = 2.0f;

    for (const auto& p : path) {
        Vec2f pos = coords.gridToScreen(p);
        DrawRectangle(
            static_cast<int>(pos.x + pad), static_cast<int>(pos.y + pad),
            static_cast<int>(cellSize - 2 * pad), static_cast<int>(cellSize - 2 * pad),
            {0, 215, 255, 220} // cyan
        );
    }
}

void Renderer::drawStartGoal(Vec2i start, Vec2i goal, const CoordinateSystem& coords) {
    float cellSize = coords.getCellSize();

    Vec2f sPos = coords.gridToScreen(start);
    DrawRectangle(
        static_cast<int>(sPos.x), static_cast<int>(sPos.y),
        static_cast<int>(cellSize), static_cast<int>(cellSize),
        {0, 200, 0, 255} // green
    );

    Vec2f gPos = coords.gridToScreen(goal);
    DrawRectangle(
        static_cast<int>(gPos.x), static_cast<int>(gPos.y),
        static_cast<int>(cellSize), static_cast<int>(cellSize),
        {200, 0, 0, 255} // red
    );
}

void Renderer::drawPath(const std::vector<Vec2i>& path, const CoordinateSystem& coords) {
    float cellSize = coords.getCellSize();
    float pad = 2.0f;

    for (const auto& p : path) {
        Vec2f pos = coords.gridToScreen(p);
        DrawRectangle(
            static_cast<int>(pos.x + pad), static_cast<int>(pos.y + pad),
            static_cast<int>(cellSize - 2 * pad), static_cast<int>(cellSize - 2 * pad),
            {255, 215, 0, 220} // gold
        );
    }
}
