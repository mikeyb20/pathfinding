#include "Algorithms/BFS.h"

PathResult BFS::findPath(const Grid& /*grid*/, Vec2i /*start*/, Vec2i /*goal*/) {
    // Stub — full implementation in Phase 1
    return PathResult{};
}

void BFS::initSearch(const Grid& /*grid*/, Vec2i /*start*/, Vec2i /*goal*/) {
    state_ = SearchState{};
}

bool BFS::step() {
    state_.finished = true;
    state_.pathFound = false;
    return false;
}
