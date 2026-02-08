#pragma once

#include "Algorithms/IPathfinder.h"

class AlgorithmAnimator {
public:
    void setPathfinder(IPathfinder* pathfinder);
    void start(const Grid& grid, Vec2i start, Vec2i goal);
    void update(float dt);
    void reset();

    void setStepsPerSecond(float sps) { stepsPerSecond_ = sps; }
    float getStepsPerSecond() const { return stepsPerSecond_; }

    void setPaused(bool paused) { paused_ = paused; }
    bool isPaused() const { return paused_; }
    bool isRunning() const { return running_; }
    bool isFinished() const;

    const SearchState& getState() const;

private:
    IPathfinder* pathfinder_ = nullptr;
    float stepsPerSecond_ = 30.0f;
    float accumulator_ = 0.0f;
    bool paused_ = false;
    bool running_ = false;
    bool hasSearchData_ = false;
};
