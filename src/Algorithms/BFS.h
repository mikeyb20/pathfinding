#pragma once

#include <queue>
#include <unordered_set>
#include <unordered_map>
#include "Algorithms/IPathfinder.h"

class BFS : public IPathfinder {
public:
    PathResult findPath(const Grid& grid, Vec2i start, Vec2i goal) override;
    std::string getName() const override { return "BFS"; }

    void initSearch(const Grid& grid, Vec2i start, Vec2i goal) override;
    bool step() override;
    const SearchState& getCurrentState() const override { return state_; }
    void setDiagonalMovement(bool enabled) override { use8Dir_ = enabled; }

private:
    bool use8Dir_ = false;
    std::vector<Vec2i> reconstructPath() const;

    SearchState state_;

    std::queue<Vec2i> queue_;
    std::unordered_set<Vec2i, Vec2iHash> visited_;
    std::unordered_map<Vec2i, Vec2i, Vec2iHash> parent_;
    const Grid* grid_ = nullptr;
    Vec2i start_;
    Vec2i goal_;
};
