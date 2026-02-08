#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "Core/Grid.h"
#include "Core/Types.h"
#include "Algorithms/IPathfinder.h"
#include "Benchmarking/BenchmarkRunner.h"

struct Scenario {
    std::string name;
    std::string description;
    nlohmann::json gridData;
    Vec2i start, goal;
};

class ScenarioManager {
public:
    void init();
    bool saveScenario(const Grid& grid, Vec2i start, Vec2i goal,
                      const std::string& name, const std::string& desc);
    bool loadScenario(int index, Grid& grid, Vec2i& start, Vec2i& goal);
    std::vector<BenchmarkResult> runScenario(int index, const std::vector<IPathfinder*>& algorithms);
    bool drawUI(Grid& grid, Vec2i& start, Vec2i& goal, const std::vector<IPathfinder*>& algorithms);
    void refresh();

private:
    void loadBuiltInScenarios();
    void scanDiskScenarios();
    std::string scenariosDirectory() const;

    std::vector<Scenario> scenarios_;
    int selectedIndex_ = -1;
    std::vector<BenchmarkResult> lastRunResults_;

    char saveNameBuf_[128] = "";
    char saveDescBuf_[256] = "";
};
