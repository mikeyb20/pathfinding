#pragma once

#include <queue>
#include <functional>
#include <unordered_set>
#include <unordered_map>
#include "Algorithms/IPathfinder.h"
#include "Algorithms/Heuristics.h"

class AStar : public IPathfinder {
public:
    using HeuristicFunc = std::function<float(Vec2i, Vec2i)>;

    explicit AStar(HeuristicFunc h = Heuristics::manhattan) : heuristic_(std::move(h)) {}

    PathResult findPath(const Grid& grid, Vec2i start, Vec2i goal) override;
    std::string getName() const override { return "A*"; }
    void initSearch(const Grid& grid, Vec2i start, Vec2i goal) override;
    bool step() override;
    const SearchState& getCurrentState() const override { return state_; }

    void setHeuristic(HeuristicFunc h) { heuristic_ = std::move(h); }

private:
    std::vector<Vec2i> reconstructPath() const;
    void rebuildFrontierSnapshot();

    HeuristicFunc heuristic_;
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
