#include "Visualization/UIOverlay.h"
#include "imgui.h"

void UIOverlay::init() {
}

void UIOverlay::shutdown() {
}

void UIOverlay::draw(const std::string& algorithmName, const SearchState& state, const PathResult& lastResult) {
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(280, 180), ImGuiCond_FirstUseEver);

    ImGui::Begin("Pathfinder Stats");

    ImGui::Text("Algorithm: %s", algorithmName.c_str());
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

    ImGui::End();
}
