#include "Algorithms/AStar.h"
#include <chrono>
#include <algorithm>

void AStar::initSearch(const Grid& grid, Vec2i start, Vec2i goal) {
    state_ = SearchState{};
    grid_ = &grid;
    start_ = start;
    goal_ = goal;

    // Clear persistent structures
    pq_ = decltype(pq_)();
    gCost_.clear();
    closed_.clear();
    parent_.clear();
    inFrontier_.clear();

    // Seed with start node (f = 0 + h)
    gCost_[start] = 0.0f;
    pq_.push({heuristic_(start, goal), start});
    inFrontier_.insert(start);
    state_.frontier = {start};
}

bool AStar::step() {
    // Skip already-closed nodes (lazy deletion)
    while (!pq_.empty() && closed_.count(pq_.top().second)) {
        inFrontier_.erase(pq_.top().second);
        pq_.pop();
    }

    if (pq_.empty()) {
        state_.finished = true;
        state_.pathFound = false;
        return false;
    }

    auto [fCost, current] = pq_.top();
    pq_.pop();
    inFrontier_.erase(current);
    closed_.insert(current);
    state_.visitedOrder.push_back(current);

    // Goal reached
    if (current == goal_) {
        state_.currentPath = reconstructPath();
        state_.pathFound = true;
        state_.finished = true;
        rebuildFrontierSnapshot();
        return false;
    }

    // Expand neighbors
    auto neighbors = use8Dir_ ? grid_->getNeighbors8(current.x, current.y)
                              : grid_->getNeighbors4(current.x, current.y);
    for (const auto& neighbor : neighbors) {
        if (closed_.count(neighbor)) continue;

        float moveCost = grid_->getCell(neighbor.x, neighbor.y).movementCost;
        if (use8Dir_ && neighbor.x != current.x && neighbor.y != current.y)
            moveCost *= 1.41421356f;
        float newG = gCost_[current] + moveCost;
        auto it = gCost_.find(neighbor);
        if (it == gCost_.end() || newG < it->second) {
            gCost_[neighbor] = newG;
            parent_[neighbor] = current;
            float f = newG + heuristic_(neighbor, goal_);
            pq_.push({f, neighbor});
            inFrontier_.insert(neighbor);
        }
    }

    rebuildFrontierSnapshot();
    return true;
}

void AStar::rebuildFrontierSnapshot() {
    state_.frontier.clear();
    state_.frontier.reserve(inFrontier_.size());
    for (const auto& v : inFrontier_) {
        state_.frontier.push_back(v);
    }
}

std::vector<Vec2i> AStar::reconstructPath() const {
    std::vector<Vec2i> path;
    Vec2i current = goal_;
    while (current != start_) {
        path.push_back(current);
        auto it = parent_.find(current);
        if (it == parent_.end()) return {};
        current = it->second;
    }
    path.push_back(start_);
    std::reverse(path.begin(), path.end());
    return path;
}

PathResult AStar::findPath(const Grid& grid, Vec2i start, Vec2i goal) {
    auto t0 = std::chrono::high_resolution_clock::now();

    initSearch(grid, start, goal);
    while (step()) {}

    auto t1 = std::chrono::high_resolution_clock::now();
    float ms = std::chrono::duration<float, std::milli>(t1 - t0).count();

    PathResult result;
    result.path = state_.currentPath;
    result.nodesExpanded = static_cast<int>(state_.visitedOrder.size());
    result.totalCost = state_.pathFound ? gCost_.at(goal_) : 0.0f;
    result.computeTimeMs = ms;
    result.visitedOrder = state_.visitedOrder;
    return result;
}
