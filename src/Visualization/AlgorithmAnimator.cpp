#include "Visualization/AlgorithmAnimator.h"

void AlgorithmAnimator::setPathfinder(IPathfinder* pathfinder) {
    pathfinder_ = pathfinder;
}

void AlgorithmAnimator::start(const Grid& grid, Vec2i start, Vec2i goal) {
    if (!pathfinder_) return;
    pathfinder_->initSearch(grid, start, goal);
    running_ = true;
    paused_ = false;
    accumulator_ = 0.0f;
}

void AlgorithmAnimator::update(float dt) {
    if (!running_ || paused_ || !pathfinder_) return;

    accumulator_ += dt;
    float stepInterval = 1.0f / stepsPerSecond_;

    while (accumulator_ >= stepInterval) {
        accumulator_ -= stepInterval;
        bool continueSearch = pathfinder_->step();
        if (!continueSearch) {
            running_ = false;
            break;
        }
    }
}

void AlgorithmAnimator::reset() {
    running_ = false;
    paused_ = false;
    accumulator_ = 0.0f;
}

bool AlgorithmAnimator::isFinished() const {
    if (!pathfinder_) return true;
    return !running_ && pathfinder_->getCurrentState().finished;
}

const SearchState& AlgorithmAnimator::getState() const {
    static SearchState empty;
    if (!pathfinder_) return empty;
    return pathfinder_->getCurrentState();
}
