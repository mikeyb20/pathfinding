#pragma once

#include <queue>
#include <functional>
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include "Algorithms/IPathfinder.h"
#include "Algorithms/Heuristics.h"

class JumpPointSearch : public IPathfinder {
public:
    using HeuristicFunc = std::function<float(Vec2i, Vec2i)>;

    explicit JumpPointSearch(HeuristicFunc h = Heuristics::octile) : heuristic_(std::move(h)) {}

    PathResult findPath(const Grid& grid, Vec2i start, Vec2i goal) override;
    std::string getName() const override { return "JPS"; }
    void initSearch(const Grid& grid, Vec2i start, Vec2i goal) override;
    bool step() override;
    const SearchState& getCurrentState() const override { return state_; }

    void setDiagonalMovement(bool enabled) override { use8Dir_ = enabled; }

private:
    std::vector<Vec2i> identifySuccessors(Vec2i node);
    std::optional<Vec2i> jump(Vec2i pos, int dx, int dy);
    std::optional<Vec2i> jumpCardinal(Vec2i pos, int dx, int dy);
    std::vector<Vec2i> reconstructPath() const;
    void rebuildFrontierSnapshot();

    HeuristicFunc heuristic_;
    bool use8Dir_ = true;
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
