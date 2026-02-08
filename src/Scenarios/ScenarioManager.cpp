#include "Scenarios/ScenarioManager.h"
#include "imgui.h"
#include <fstream>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

std::string ScenarioManager::scenariosDirectory() const {
    std::string dir = "data/scenarios/";
    fs::create_directories(dir);
    return dir;
}

void ScenarioManager::init() {
    loadBuiltInScenarios();
    scanDiskScenarios();
}

void ScenarioManager::refresh() {
    scenarios_.clear();
    selectedIndex_ = -1;
    lastRunResults_.clear();
    init();
}

void ScenarioManager::loadBuiltInScenarios() {
    // 1. Maze — corridor walls testing narrow passages
    {
        Scenario s;
        s.name = "Maze";
        s.description = "Corridor walls testing narrow passages";
        s.start = {1, 1};
        s.goal = {38, 23};

        Grid g(40, 25);
        // Create a simple maze with vertical walls and gaps
        for (int x = 5; x < 40; x += 5) {
            for (int y = 0; y < 25; ++y) {
                g.setTerrain(x, y, TerrainType::Wall);
            }
            // Leave gap at alternating positions
            int gapY = (x / 5 % 2 == 1) ? 3 : 21;
            g.setTerrain(x, gapY, TerrainType::Open);
            g.setTerrain(x, gapY + 1, TerrainType::Open);
        }

        s.gridData = g.toJson();
        scenarios_.push_back(s);
    }

    // 2. Open Field — sparse obstacles
    {
        Scenario s;
        s.name = "Open Field";
        s.description = "Sparse obstacles testing algorithm efficiency";
        s.start = {1, 12};
        s.goal = {38, 12};

        Grid g(40, 25);
        // A few scattered wall clusters
        for (int i = 0; i < 8; ++i) {
            int bx = 5 + i * 4;
            int by = 8 + (i % 3) * 4;
            for (int dy = 0; dy < 3; ++dy)
                for (int dx = 0; dx < 2; ++dx)
                    if (g.inBounds(bx + dx, by + dy))
                        g.setTerrain(bx + dx, by + dy, TerrainType::Wall);
        }

        s.gridData = g.toJson();
        scenarios_.push_back(s);
    }

    // 3. Weighted Terrain — mix of water/mud/forest
    {
        Scenario s;
        s.name = "Weighted Terrain";
        s.description = "Mix of water, mud, and forest demonstrating cost-awareness";
        s.start = {1, 1};
        s.goal = {38, 23};

        Grid g(40, 25);
        // Water band across the middle
        for (int x = 0; x < 40; ++x)
            for (int y = 10; y < 15; ++y)
                g.setTerrain(x, y, TerrainType::Water);
        // Mud patches
        for (int x = 5; x < 15; ++x)
            for (int y = 3; y < 8; ++y)
                g.setTerrain(x, y, TerrainType::Mud);
        for (int x = 25; x < 35; ++x)
            for (int y = 17; y < 22; ++y)
                g.setTerrain(x, y, TerrainType::Mud);
        // Forest edges
        for (int x = 15; x < 25; ++x) {
            for (int y = 0; y < 5; ++y)
                g.setTerrain(x, y, TerrainType::Forest);
            for (int y = 20; y < 25; ++y)
                g.setTerrain(x, y, TerrainType::Forest);
        }
        // Bridge through water (open corridor)
        for (int y = 10; y < 15; ++y)
            g.setTerrain(20, y, TerrainType::Open);

        s.gridData = g.toJson();
        scenarios_.push_back(s);
    }
}

void ScenarioManager::scanDiskScenarios() {
    std::string dir = scenariosDirectory();
    if (!fs::exists(dir)) return;

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.path().extension() != ".json") continue;

        std::ifstream in(entry.path());
        if (!in.is_open()) continue;

        nlohmann::json j;
        try {
            in >> j;
        } catch (...) {
            continue;
        }

        Scenario s;
        s.name = j.value("name", entry.path().stem().string());
        s.description = j.value("description", "");
        if (j.contains("start")) {
            s.start.x = j["start"]["x"].get<int>();
            s.start.y = j["start"]["y"].get<int>();
        }
        if (j.contains("goal")) {
            s.goal.x = j["goal"]["x"].get<int>();
            s.goal.y = j["goal"]["y"].get<int>();
        }
        if (j.contains("grid")) {
            s.gridData = j["grid"];
        }
        scenarios_.push_back(s);
    }
}

bool ScenarioManager::saveScenario(const Grid& grid, Vec2i start, Vec2i goal,
                                   const std::string& name, const std::string& desc) {
    nlohmann::json j;
    j["version"] = 1;
    j["name"] = name;
    j["description"] = desc;
    j["start"] = {{"x", start.x}, {"y", start.y}};
    j["goal"] = {{"x", goal.x}, {"y", goal.y}};
    j["grid"] = grid.toJson();

    // Sanitize filename from name
    std::string filename = name;
    for (char& c : filename) {
        if (!std::isalnum(c) && c != '-' && c != '_')
            c = '_';
    }

    std::string path = scenariosDirectory() + filename + ".json";
    std::ofstream out(path);
    if (!out.is_open()) return false;
    out << j.dump(2);
    return out.good();
}

bool ScenarioManager::loadScenario(int index, Grid& grid, Vec2i& start, Vec2i& goal) {
    if (index < 0 || index >= static_cast<int>(scenarios_.size())) return false;

    const auto& s = scenarios_[index];
    grid.fromJson(s.gridData);
    start = s.start;
    goal = s.goal;
    return true;
}

std::vector<BenchmarkResult> ScenarioManager::runScenario(int index,
                                                          const std::vector<IPathfinder*>& algorithms) {
    std::vector<BenchmarkResult> results;
    if (index < 0 || index >= static_cast<int>(scenarios_.size())) return results;

    const auto& s = scenarios_[index];
    Grid tempGrid(40, 25);
    tempGrid.fromJson(s.gridData);

    BenchmarkRunner runner;
    runner.run(tempGrid, s.start, s.goal, algorithms, 50);
    return runner.getResults();
}

bool ScenarioManager::drawUI(Grid& grid, Vec2i& start, Vec2i& goal,
                             const std::vector<IPathfinder*>& algorithms) {
    bool gridChanged = false;

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Scenarios");
    ImGui::Separator();

    if (!scenarios_.empty()) {
        if (ImGui::BeginListBox("##Scenarios", ImVec2(-1, ImGui::GetTextLineHeightWithSpacing() * 6))) {
            for (int i = 0; i < static_cast<int>(scenarios_.size()); ++i) {
                bool selected = (i == selectedIndex_);
                if (ImGui::Selectable(scenarios_[i].name.c_str(), selected)) {
                    selectedIndex_ = i;
                }
                if (ImGui::IsItemHovered() && !scenarios_[i].description.empty()) {
                    ImGui::SetTooltip("%s", scenarios_[i].description.c_str());
                }
            }
            ImGui::EndListBox();
        }

        if (ImGui::Button("Load") && selectedIndex_ >= 0) {
            if (loadScenario(selectedIndex_, grid, start, goal)) {
                gridChanged = true;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Run All") && selectedIndex_ >= 0) {
            lastRunResults_ = runScenario(selectedIndex_, algorithms);
        }
    } else {
        ImGui::TextDisabled("No scenarios available");
    }

    // Results table
    if (!lastRunResults_.empty()) {
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Comparison");
        ImGui::Separator();

        if (ImGui::BeginTable("ScenarioResults", 6,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit)) {
            ImGui::TableSetupColumn("Algorithm");
            ImGui::TableSetupColumn("Found");
            ImGui::TableSetupColumn("Cost");
            ImGui::TableSetupColumn("Nodes");
            ImGui::TableSetupColumn("Path");
            ImGui::TableSetupColumn("Time (ms)");
            ImGui::TableHeadersRow();

            for (const auto& r : lastRunResults_) {
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

    // Save current as scenario
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Save Current as Scenario");
    ImGui::Separator();

    ImGui::InputText("Name##scen", saveNameBuf_, sizeof(saveNameBuf_));
    ImGui::InputTextMultiline("Description##scen", saveDescBuf_, sizeof(saveDescBuf_),
                              ImVec2(-1, ImGui::GetTextLineHeight() * 3));

    if (ImGui::Button("Save Scenario") && saveNameBuf_[0] != '\0') {
        if (saveScenario(grid, start, goal, saveNameBuf_, saveDescBuf_)) {
            refresh();
        }
    }

    return gridChanged;
}
