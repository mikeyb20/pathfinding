#include "Benchmarking/BenchmarkRunner.h"
#include "imgui.h"
#define ANKERL_NANOBENCH_IMPLEMENT
#include <nanobench.h>
#include <sstream>

void BenchmarkRunner::run(const Grid& grid, Vec2i start, Vec2i goal,
                          const std::vector<IPathfinder*>& algorithms, int iterations) {
    results_.clear();

    for (auto* algo : algorithms) {
        BenchmarkResult result;
        result.algorithmName = algo->getName();

        // Run one warm-up pass to get the PathResult
        PathResult pr = algo->findPath(grid, start, goal);
        result.pathFound = pr.found();
        result.pathCost = pr.totalCost;
        result.nodesExpanded = pr.nodesExpanded;
        result.pathLength = static_cast<int>(pr.path.size());

        // Benchmark timing with nanobench
        double medianNs = 0.0;
        ankerl::nanobench::Bench bench;
        bench.minEpochIterations(iterations)
             .warmup(3)
             .output(nullptr);

        bench.run(algo->getName(), [&] {
            algo->findPath(grid, start, goal);
        });

        // Extract median time from results
        for (const auto& r : bench.results()) {
            medianNs = r.median(ankerl::nanobench::Result::Measure::elapsed);
        }
        result.computeTimeMs = medianNs * 1000.0; // nanobench returns seconds

        results_.push_back(result);
    }
}

bool BenchmarkRunner::drawUI() {
    runRequested_ = false;

    ImGui::SliderInt("Iterations", &iterations_, 10, 1000);

    if (ImGui::Button("Run Benchmark")) {
        runRequested_ = true;
    }

    if (!results_.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Results");
        ImGui::Separator();

        if (ImGui::BeginTable("BenchResults", 6,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Algorithm");
            ImGui::TableSetupColumn("Found");
            ImGui::TableSetupColumn("Cost");
            ImGui::TableSetupColumn("Nodes");
            ImGui::TableSetupColumn("Path");
            ImGui::TableSetupColumn("Time (ms)");
            ImGui::TableHeadersRow();

            for (const auto& r : results_) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); ImGui::Text("%s", r.algorithmName.c_str());
                ImGui::TableNextColumn(); ImGui::Text("%s", r.pathFound ? "Yes" : "No");
                ImGui::TableNextColumn(); ImGui::Text("%.2f", r.pathCost);
                ImGui::TableNextColumn(); ImGui::Text("%d", r.nodesExpanded);
                ImGui::TableNextColumn(); ImGui::Text("%d", r.pathLength);
                ImGui::TableNextColumn(); ImGui::Text("%.3f", r.computeTimeMs);
            }
            ImGui::EndTable();
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
