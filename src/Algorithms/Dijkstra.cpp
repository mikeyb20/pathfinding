#include "Algorithms/Dijkstra.h"

PathResult Dijkstra::findPath(const Grid& /*grid*/, Vec2i /*start*/, Vec2i /*goal*/) {
    // Stub — full implementation in Phase 2
    return PathResult{};
}

void Dijkstra::initSearch(const Grid& /*grid*/, Vec2i /*start*/, Vec2i /*goal*/) {
    state_ = SearchState{};
}

bool Dijkstra::step() {
    state_.finished = true;
    state_.pathFound = false;
    return false;
}
