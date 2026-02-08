#pragma once

#include <queue>
#include <unordered_set>
#include <unordered_map>
#include "Algorithms/IPathfinder.h"

class Dijkstra : public IPathfinder {
public:
    PathResult findPath(const Grid& grid, Vec2i start, Vec2i goal) override;
    std::string getName() const override { return "Dijkstra"; }
    void initSearch(const Grid& grid, Vec2i start, Vec2i goal) override;
    bool step() override;
    const SearchState& getCurrentState() const override { return state_; }
    void setDiagonalMovement(bool enabled) override { use8Dir_ = enabled; }

private:
    bool use8Dir_ = false;
    std::vector<Vec2i> reconstructPath() const;
    void rebuildFrontierSnapshot();

    SearchState state_;
    using PQEntry = std::pair<float, Vec2i>;
    struct PQCompare {
        bool operator()(const PQEntry& a, const PQEntry& b) const {
            return a.first > b.first;
        }
    };
    std::priority_queue<PQEntry, std::vector<PQEntry>, PQCompare> pq_;
    std::unordered_map<Vec2i, float, Vec2iHash> gCost_;
    std::unordered_set<Vec2i, Vec2iHash> closed_;
    std::unordered_map<Vec2i, Vec2i, Vec2iHash> parent_;
    std::unordered_set<Vec2i, Vec2iHash> inFrontier_;
    const Grid* grid_ = nullptr;
    Vec2i start_, goal_;
};
