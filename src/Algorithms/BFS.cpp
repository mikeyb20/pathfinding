#include "Algorithms/BFS.h"
#include <chrono>
#include <algorithm>

void BFS::initSearch(const Grid& grid, Vec2i start, Vec2i goal) {
    state_ = SearchState{};
    grid_ = &grid;
    start_ = start;
    goal_ = goal;

    // Clear persistent structures
    queue_ = std::queue<Vec2i>();
    visited_.clear();
    parent_.clear();

    // Seed with start node
    queue_.push(start);
    visited_.insert(start);
    state_.frontier = {start};
}

bool BFS::step() {
    if (queue_.empty()) {
        state_.finished = true;
        state_.pathFound = false;
        return false;
    }

    Vec2i current = queue_.front();
    queue_.pop();

    state_.visitedOrder.push_back(current);

    // Goal reached
    if (current == goal_) {
        state_.currentPath = reconstructPath();
        state_.pathFound = true;
        state_.finished = true;
        return false;
    }

    // Expand neighbors
    auto neighbors = grid_->getNeighbors4(current.x, current.y);
    for (const auto& neighbor : neighbors) {
        if (visited_.find(neighbor) == visited_.end()) {
            visited_.insert(neighbor);
            parent_[neighbor] = current;
            queue_.push(neighbor);
        }
    }

    // Snapshot frontier for visualization
    state_.frontier.clear();
    std::queue<Vec2i> copy = queue_;
    while (!copy.empty()) {
        state_.frontier.push_back(copy.front());
        copy.pop();
    }

    return true;
}

std::vector<Vec2i> BFS::reconstructPath() const {
    std::vector<Vec2i> path;
    Vec2i current = goal_;
    while (current != start_) {
        path.push_back(current);
        auto it = parent_.find(current);
        if (it == parent_.end()) return {}; // no path (shouldn't happen if called correctly)
        current = it->second;
    }
    path.push_back(start_);
    std::reverse(path.begin(), path.end());
    return path;
}

PathResult BFS::findPath(const Grid& grid, Vec2i start, Vec2i goal) {
    auto t0 = std::chrono::high_resolution_clock::now();

    initSearch(grid, start, goal);
    while (step()) {}

    auto t1 = std::chrono::high_resolution_clock::now();
    float ms = std::chrono::duration<float, std::milli>(t1 - t0).count();

    PathResult result;
    result.path = state_.currentPath;
    result.nodesExpanded = static_cast<int>(state_.visitedOrder.size());
    result.totalCost = state_.pathFound ? static_cast<float>(state_.currentPath.size() - 1) : 0.0f;
    result.computeTimeMs = ms;
    result.visitedOrder = state_.visitedOrder;
    return result;
}
