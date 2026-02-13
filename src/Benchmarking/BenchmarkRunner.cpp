#include "Benchmarking/BenchmarkRunner.h"
#include "imgui.h"
#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>
#include <sstream>

void BenchmarkRunner::run(const Grid& grid, Vec2i start, Vec2i goal,
                          const std::vector<IPathfinder*>& algorithms, int iterations) {
    startRun(grid, start, goal, algorithms, iterations);
    while (tick()) {}
}

void BenchmarkRunner::startRun(const Grid& grid, Vec2i start, Vec2i goal,
                               const std::vector<IPathfinder*>& algorithms, int iterations) {
    results_.clear();
    benchGrid_ = &grid;
    benchStart_ = start;
    benchGoal_ = goal;
    benchAlgos_ = &algorithms;
    iterations_ = iterations;
    currentAlgoIndex_ = 0;
    totalAlgos_ = static_cast<int>(algorithms.size());
    running_ = (totalAlgos_ > 0);
}

bool BenchmarkRunner::tick() {
    if (!running_ || !benchAlgos_ || currentAlgoIndex_ >= totalAlgos_) {
        running_ = false;
        return false;
    }

    auto* algo = (*benchAlgos_)[currentAlgoIndex_];
    BenchmarkResult result;
    result.algorithmName = algo->getName();

    // Run one warm-up pass to get the PathResult
    PathResult pr = algo->findPath(*benchGrid_, benchStart_, benchGoal_);
    result.pathFound = pr.found();
    result.pathCost = pr.totalCost;
    result.nodesExpanded = pr.nodesExpanded;
    result.pathLength = static_cast<int>(pr.path.size());

    // Benchmark timing with nanobench
    double medianNs = 0.0;
    ankerl::nanobench::Bench bench;
    bench.minEpochIterations(iterations_)
         .warmup(3)
         .output(nullptr);

    bench.run(algo->getName(), [&] {
        algo->findPath(*benchGrid_, benchStart_, benchGoal_);
    });

    // Extract median time from results
    for (const auto& r : bench.results()) {
        medianNs = r.median(ankerl::nanobench::Result::Measure::elapsed);
    }
    result.computeTimeMs = medianNs * 1000.0; // nanobench returns seconds

    results_.push_back(result);
    ++currentAlgoIndex_;

    if (currentAlgoIndex_ >= totalAlgos_) {
        running_ = false;
        return false;
    }
    return true;
}

float BenchmarkRunner::progress() const {
    if (totalAlgos_ == 0) return 0.0f;
    return static_cast<float>(currentAlgoIndex_) / static_cast<float>(totalAlgos_);
}

std::string BenchmarkRunner::currentAlgorithmName() const {
    if (!running_ || !benchAlgos_ || currentAlgoIndex_ >= totalAlgos_) return "";
    return (*benchAlgos_)[currentAlgoIndex_]->getName();
}

bool BenchmarkRunner::drawUI() {
    runRequested_ = false;

    ImGui::SliderInt("Iterations", &iterations_, 10, 1000);

    // Disable button while running
    if (running_) {
        ImGui::BeginDisabled();
    }
    if (ImGui::Button("Run Benchmark")) {
        runRequested_ = true;
    }
    if (running_) {
        ImGui::EndDisabled();
    }

    // Progress indicator while running
    if (running_) {
        ImGui::Spacing();
        ImGui::ProgressBar(progress());
        std::string label = "Benchmarking " + currentAlgorithmName() + "...";
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "%s", label.c_str());
    }

    // Results as vertical cards
    if (!results_.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Results");
        ImGui::Separator();

        for (size_t i = 0; i < results_.size(); ++i) {
            const auto& r = results_[i];

            ImGui::TextColored(ImVec4(0.4f, 0.85f, 1.0f, 1.0f), "[%s]", r.algorithmName.c_str());
            ImGui::Text("  Found:    %s", r.pathFound ? "Yes" : "No");
            ImGui::Text("  Cost:     %.2f", r.pathCost);
            ImGui::Text("  Nodes:    %d", r.nodesExpanded);
            ImGui::Text("  Path:     %d", r.pathLength);
            ImGui::Text("  Time:     %.3f ms", r.computeTimeMs);

            if (i + 1 < results_.size()) {
                ImGui::Separator();
            }
        }
    }

    return runRequested_;
}

nlohmann::json BenchmarkRunner::resultsToJson() const {
    nlohmann::json j = nlohmann::json::array();
    for (const auto& r : results_) {
        j.push_back({
            {"algorithm", r.algorithmName},
            {"pathFound", r.pathFound},
            {"pathCost", r.pathCost},
            {"nodesExpanded", r.nodesExpanded},
            {"pathLength", r.pathLength},
            {"computeTimeMs", r.computeTimeMs}
        });
    }
    return j;
}
