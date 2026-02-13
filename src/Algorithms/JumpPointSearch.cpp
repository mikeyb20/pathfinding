#include "Algorithms/JumpPointSearch.h"
#include <chrono>
#include <algorithm>
#include <cmath>

static constexpr float SQRT2 = 1.41421356f;

// Octile distance between two points (uniform cost)
static float octileDistance(Vec2i a, Vec2i b) {
    float dx = static_cast<float>(std::abs(a.x - b.x));
    float dy = static_cast<float>(std::abs(a.y - b.y));
    return std::max(dx, dy) + (SQRT2 - 1.0f) * std::min(dx, dy);
}

void JumpPointSearch::initSearch(const Grid& grid, Vec2i start, Vec2i goal) {
    state_ = SearchState{};
    grid_ = &grid;
    start_ = start;
    goal_ = goal;

    pq_ = decltype(pq_)();
    gCost_.clear();
    closed_.clear();
    parent_.clear();
    inFrontier_.clear();

    gCost_[start] = 0.0f;
    pq_.push({heuristic_(start, goal), start});
    inFrontier_.insert(start);
    state_.frontier = {start};
}

bool JumpPointSearch::step() {
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

    if (current == goal_) {
        state_.currentPath = reconstructPath();
        state_.pathFound = true;
        state_.finished = true;
        rebuildFrontierSnapshot();
        return false;
    }

    auto successors = identifySuccessors(current);
    for (const auto& jp : successors) {
        if (closed_.count(jp)) continue;

        float newG = gCost_[current] + octileDistance(current, jp);
        auto it = gCost_.find(jp);
        if (it == gCost_.end() || newG < it->second) {
            gCost_[jp] = newG;
            parent_[jp] = current;
            float f = newG + heuristic_(jp, goal_);
            pq_.push({f, jp});
            inFrontier_.insert(jp);
        }
    }

    rebuildFrontierSnapshot();
    return true;
}

std::vector<Vec2i> JumpPointSearch::identifySuccessors(Vec2i node) {
    std::vector<Vec2i> successors;

    if (node == start_) {
        // From start, try all 8 directions
        static const int dirs[8][2] = {
            {0,-1}, {1,0}, {0,1}, {-1,0},
            {1,-1}, {1,1}, {-1,1}, {-1,-1}
        };
        for (auto& d : dirs) {
            auto jp = jump(node, d[0], d[1]);
            if (jp) successors.push_back(*jp);
        }
        return successors;
    }

    // Determine parent direction
    auto pit = parent_.find(node);
    if (pit == parent_.end()) return successors;

    Vec2i par = pit->second;
    int dx = 0, dy = 0;
    if (node.x - par.x > 0) dx = 1;
    else if (node.x - par.x < 0) dx = -1;
    if (node.y - par.y > 0) dy = 1;
    else if (node.y - par.y < 0) dy = -1;

    auto walkable = [&](int x, int y) {
        return grid_->inBounds(x, y) && grid_->isWalkable(x, y);
    };

    int x = node.x, y = node.y;

    if (dx != 0 && dy != 0) {
        // Diagonal move: natural neighbors are (dx,dy), (dx,0), (0,dy)
        // Always try the three natural directions
        auto jp1 = jump(node, dx, dy);
        if (jp1) successors.push_back(*jp1);
        auto jp2 = jump(node, dx, 0);
        if (jp2) successors.push_back(*jp2);
        auto jp3 = jump(node, 0, dy);
        if (jp3) successors.push_back(*jp3);

        // Forced neighbors for diagonal:
        // If (x - dx, y) is blocked, then (x - dx, y + dy) is forced
        if (!walkable(x - dx, y)) {
            auto jp = jump(node, -dx, dy);
            if (jp) successors.push_back(*jp);
        }
        // If (x, y - dy) is blocked, then (x + dx, y - dy) is forced
        if (!walkable(x, y - dy)) {
            auto jp = jump(node, dx, -dy);
            if (jp) successors.push_back(*jp);
        }
    } else {
        // Cardinal move
        if (dx != 0) {
            // Moving horizontally
            // Natural: continue in same direction
            auto jp1 = jump(node, dx, 0);
            if (jp1) successors.push_back(*jp1);

            // Forced: if (x, y-1) blocked, (x+dx, y-1) is forced
            if (!walkable(x, y - 1)) {
                auto jp = jump(node, dx, -1);
                if (jp) successors.push_back(*jp);
            }
            // Forced: if (x, y+1) blocked, (x+dx, y+1) is forced
            if (!walkable(x, y + 1)) {
                auto jp = jump(node, dx, 1);
                if (jp) successors.push_back(*jp);
            }
        } else {
            // Moving vertically
            // Natural: continue in same direction
            auto jp1 = jump(node, 0, dy);
            if (jp1) successors.push_back(*jp1);

            // Forced: if (x-1, y) blocked, (x-1, y+dy) is forced
            if (!walkable(x - 1, y)) {
                auto jp = jump(node, -1, dy);
                if (jp) successors.push_back(*jp);
            }
            // Forced: if (x+1, y) blocked, (x+1, y+dy) is forced
            if (!walkable(x + 1, y)) {
                auto jp = jump(node, 1, dy);
                if (jp) successors.push_back(*jp);
            }
        }
    }

    return successors;
}

std::optional<Vec2i> JumpPointSearch::jumpCardinal(Vec2i pos, int dx, int dy) {
    while (true) {
        int nx = pos.x + dx;
        int ny = pos.y + dy;

        if (!grid_->inBounds(nx, ny) || !grid_->isWalkable(nx, ny))
            return std::nullopt;

        Vec2i next = {nx, ny};
        if (next == goal_) return next;

        if (dx != 0) {
            // Horizontal: forced if y±1 blocked but diagonal ahead walkable
            if ((!grid_->inBounds(nx, ny - 1) || !grid_->isWalkable(nx, ny - 1))
                && grid_->inBounds(nx + dx, ny - 1) && grid_->isWalkable(nx + dx, ny - 1))
                return next;
            if ((!grid_->inBounds(nx, ny + 1) || !grid_->isWalkable(nx, ny + 1))
                && grid_->inBounds(nx + dx, ny + 1) && grid_->isWalkable(nx + dx, ny + 1))
                return next;
        } else {
            // Vertical: forced if x±1 blocked but diagonal ahead walkable
            if ((!grid_->inBounds(nx - 1, ny) || !grid_->isWalkable(nx - 1, ny))
                && grid_->inBounds(nx - 1, ny + dy) && grid_->isWalkable(nx - 1, ny + dy))
                return next;
            if ((!grid_->inBounds(nx + 1, ny) || !grid_->isWalkable(nx + 1, ny))
                && grid_->inBounds(nx + 1, ny + dy) && grid_->isWalkable(nx + 1, ny + dy))
                return next;
        }

        pos = next;
    }
}

std::optional<Vec2i> JumpPointSearch::jump(Vec2i pos, int dx, int dy) {
    // Cardinal directions use the iterative helper directly
    if (dx == 0 || dy == 0)
        return jumpCardinal(pos, dx, dy);

    // Diagonal direction: iterate with cardinal sub-scans at each step
    while (true) {
        int nx = pos.x + dx;
        int ny = pos.y + dy;

        if (!grid_->inBounds(nx, ny) || !grid_->isWalkable(nx, ny))
            return std::nullopt;

        // Corner-cutting check: both adjacent cardinal cells must be walkable
        if (!grid_->isWalkable(pos.x + dx, pos.y) || !grid_->isWalkable(pos.x, pos.y + dy))
            return std::nullopt;

        Vec2i next = {nx, ny};
        if (next == goal_) return next;

        // Diagonal forced neighbor checks
        if ((!grid_->inBounds(nx - dx, ny) || !grid_->isWalkable(nx - dx, ny))
            && grid_->inBounds(nx - dx, ny + dy) && grid_->isWalkable(nx - dx, ny + dy))
            return next;
        if ((!grid_->inBounds(nx, ny - dy) || !grid_->isWalkable(nx, ny - dy))
            && grid_->inBounds(nx + dx, ny - dy) && grid_->isWalkable(nx + dx, ny - dy))
            return next;

        // Cardinal sub-scans (iterative, no recursion)
        if (jumpCardinal(next, dx, 0))
            return next;
        if (jumpCardinal(next, 0, dy))
            return next;

        pos = next;
    }
}

std::vector<Vec2i> JumpPointSearch::reconstructPath() const {
    // Build jump-point chain
    std::vector<Vec2i> jpChain;
    Vec2i current = goal_;
    while (current != start_) {
        jpChain.push_back(current);
        auto it = parent_.find(current);
        if (it == parent_.end()) return {};
        current = it->second;
    }
    jpChain.push_back(start_);
    std::reverse(jpChain.begin(), jpChain.end());

    // Interpolate between jump points to get cell-by-cell path
    std::vector<Vec2i> path;
    path.push_back(jpChain[0]);

    for (size_t i = 1; i < jpChain.size(); ++i) {
        Vec2i from = jpChain[i - 1];
        Vec2i to = jpChain[i];

        int dx = 0, dy = 0;
        if (to.x > from.x) dx = 1;
        else if (to.x < from.x) dx = -1;
        if (to.y > from.y) dy = 1;
        else if (to.y < from.y) dy = -1;

        Vec2i step = from;
        while (step != to) {
            step.x += dx;
            step.y += dy;
            path.push_back(step);
        }
    }

    return path;
}

void JumpPointSearch::rebuildFrontierSnapshot() {
    state_.frontier.clear();
    state_.frontier.reserve(inFrontier_.size());
    for (const auto& v : inFrontier_) {
        state_.frontier.push_back(v);
    }
}

PathResult JumpPointSearch::findPath(const Grid& grid, Vec2i start, Vec2i goal) {
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
