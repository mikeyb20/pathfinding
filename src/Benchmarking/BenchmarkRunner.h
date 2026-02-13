#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "Core/Grid.h"
#include "Core/Types.h"
#include "Algorithms/IPathfinder.h"

struct BenchmarkResult {
    std::string algorithmName;
    float pathCost = 0.0f;
    int nodesExpanded = 0;
    double computeTimeMs = 0.0;
    int pathLength = 0;
    bool pathFound = false;
};

class BenchmarkRunner {
public:
    void run(const Grid& grid, Vec2i start, Vec2i goal,
             const std::vector<IPathfinder*>& algorithms, int iterations = 100);

    // Incremental benchmarking (one algorithm per frame)
    void startRun(const Grid& grid, Vec2i start, Vec2i goal,
                  const std::vector<IPathfinder*>& algorithms, int iterations = 100);
    bool tick();  // benchmarks one algorithm, returns true if still running
    bool isRunning() const { return running_; }
    float progress() const;
    std::string currentAlgorithmName() const;

    bool drawUI();  // returns true if "Run Benchmark" clicked
    const std::vector<BenchmarkResult>& getResults() const { return results_; }
    nlohmann::json resultsToJson() const;

private:
    std::vector<BenchmarkResult> results_;
    int iterations_ = 100;
    bool runRequested_ = false;

    // Incremental run state
    bool running_ = false;
    int currentAlgoIndex_ = 0;
    int totalAlgos_ = 0;
    const Grid* benchGrid_ = nullptr;
    Vec2i benchStart_, benchGoal_;
    const std::vector<IPathfinder*>* benchAlgos_ = nullptr;
};
