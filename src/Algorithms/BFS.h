#pragma once

#include "Algorithms/IPathfinder.h"

class BFS : public IPathfinder {
public:
    PathResult findPath(const Grid& grid, Vec2i start, Vec2i goal) override;
    std::string getName() const override { return "BFS"; }

    void initSearch(const Grid& grid, Vec2i start, Vec2i goal) override;
    bool step() override;
    const SearchState& getCurrentState() const override { return state_; }

private:
    SearchState state_;
};
