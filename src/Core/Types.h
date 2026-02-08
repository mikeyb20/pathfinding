#pragma once

#include <cmath>
#include <limits>
#include <string>

struct Vec2i {
    int x = 0;
    int y = 0;

    bool operator==(const Vec2i& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vec2i& other) const { return !(*this == other); }
};

struct Vec2f {
    float x = 0.0f;
    float y = 0.0f;
};

enum class TerrainType {
    Open,
    Wall,
    Water,
    Mud,
    Forest
};

inline float terrainCost(TerrainType t) {
    switch (t) {
        case TerrainType::Open:   return 1.0f;
        case TerrainType::Wall:   return std::numeric_limits<float>::infinity();
        case TerrainType::Water:  return 5.0f;
        case TerrainType::Mud:    return 3.0f;
        case TerrainType::Forest: return 2.0f;
    }
    return 1.0f;
}

inline const char* terrainToString(TerrainType t) {
    switch (t) {
        case TerrainType::Open:   return "open";
        case TerrainType::Wall:   return "wall";
        case TerrainType::Water:  return "water";
        case TerrainType::Mud:    return "mud";
        case TerrainType::Forest: return "forest";
    }
    return "open";
}

inline TerrainType terrainFromString(const std::string& s) {
    if (s == "wall")   return TerrainType::Wall;
    if (s == "water")  return TerrainType::Water;
    if (s == "mud")    return TerrainType::Mud;
    if (s == "forest") return TerrainType::Forest;
    return TerrainType::Open;
}
