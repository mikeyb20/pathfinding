#include "Visualization/UIOverlay.h"
#include "imgui.h"

static const char* terrainName(TerrainType t) {
    switch (t) {
        case TerrainType::Wall:   return "Wall";
        case TerrainType::Water:  return "Water (5x)";
        case TerrainType::Mud:    return "Mud (3x)";
        case TerrainType::Forest: return "Forest (2x)";
        case TerrainType::Open:   return "Open (1x)";
    }
    return "Unknown";
}

void UIOverlay::init() {
}

void UIOverlay::shutdown() {
}

void UIOverlay::drawPathfindingTab(const std::string& algorithmName, const SearchState& state,
                                   const PathResult& lastResult, AlgorithmAnimator& animator,
                                   const std::vector<IPathfinder*>& algorithms, int currentIndex,
                                   TerrainType currentBrush) {
    // Algorithm combo box
    if (ImGui::BeginCombo("Algorithm", algorithmName.c_str())) {
        for (int i = 0; i < static_cast<int>(algorithms.size()); ++i) {
            bool isSelected = (i == currentIndex);
            if (ImGui::Selectable(algorithms[i]->getName().c_str(), isSelected)) {
                if (i != currentIndex) {
                    requestedAlgorithm_ = i;
                }
            }
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();

    ImGui::Text("Nodes visited:  %zu", state.visitedOrder.size());
    ImGui::Text("Frontier size:  %zu", state.frontier.size());
    ImGui::Text("Path length:    %zu", state.currentPath.size());

    if (state.finished) {
        ImGui::Separator();
        if (state.pathFound) {
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "Path found!");
        } else {
            ImGui::TextColored(ImVec4(1, 0, 0, 1), "No path found");
        }
    }

    ImGui::Separator();
    ImGui::Text("Last result:");
    ImGui::Text("  Cost:     %.2f", lastResult.totalCost);
    ImGui::Text("  Expanded: %d", lastResult.nodesExpanded);
    ImGui::Text("  Time:     %.3f ms", lastResult.computeTimeMs);

    // Terrain brush indicator
    ImGui::Separator();
    ImGui::Text("Brush: %s", terrainName(currentBrush));

    // Controls
    ImGui::Separator();
    ImGui::Text("Controls");

    float sps = animator.getStepsPerSecond();
    if (ImGui::SliderFloat("Speed", &sps, 1.0f, 500.0f, "%.0f sps")) {
        animator.setStepsPerSecond(sps);
    }

    bool paused = animator.isPaused();
    if (ImGui::Button(paused ? "Play" : "Pause")) {
        animator.setPaused(!paused);
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        resetRequested_ = true;
    }

    // Keybind help
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Keybinds:");
    ImGui::TextWrapped(
        "Space: Run  |  Enter: Instant solve\n"
        "P: Pause/Resume  |  R: Reset\n"
        "S+Click: Set start  |  G+Click: Set goal\n"
        "LMB: Draw brush  |  RMB: Erase\n"
        "1: Wall  2: Water  3: Mud  4: Forest  5: Open"
    );
}

void UIOverlay::draw(const std::string& algorithmName, const SearchState& state,
                     const PathResult& lastResult, AlgorithmAnimator& animator,
                     const std::vector<IPathfinder*>& algorithms, int currentIndex,
                     TerrainType currentBrush,
                     MapEditor& mapEditor, MapMetadata& mapMeta,
                     BenchmarkRunner& benchRunner, ScenarioManager& scenarioMgr,
                     Grid& grid, Vec2i& start, Vec2i& goal) {
    ImGui::SetNextWindowPos(ImVec2(1000, 0));
    ImGui::SetNextWindowSize(ImVec2(280, 720));

    ImGui::Begin("Pathfinder", nullptr,
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse);

    if (ImGui::BeginTabBar("MainTabs")) {
        if (ImGui::BeginTabItem("Search")) {
            drawPathfindingTab(algorithmName, state, lastResult, animator,
                               algorithms, currentIndex, currentBrush);
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Map")) {
            if (mapEditor.drawUI(grid, mapMeta)) {
                mapChanged_ = true;
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Bench")) {
            if (benchRunner.drawUI()) {
                benchmarkRequested_ = true;
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Scenarios")) {
            if (scenarioMgr.drawUI(grid, start, goal, algorithms)) {
                mapChanged_ = true;
            }
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}
