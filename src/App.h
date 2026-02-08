#pragma once

#include <vector>
#include "Core/Grid.h"
#include "Core/CoordinateSystem.h"
#include "Core/PathResult.h"
#include "Algorithms/IPathfinder.h"
#include "Algorithms/BFS.h"
#include "Algorithms/Dijkstra.h"
#include "Algorithms/AStar.h"
#include "Visualization/Renderer.h"
#include "Visualization/AlgorithmAnimator.h"
#include "Visualization/UIOverlay.h"
#include "MapEditor/MapEditor.h"
#include "Benchmarking/BenchmarkRunner.h"
#include "Scenarios/ScenarioManager.h"

class App {
public:
    void init();
    void run();
    void shutdown();

private:
    void handleInput();
    void update();
    void draw();
    void switchAlgorithm(int index);
    void generateTestMap();
    void ensureStartGoalWalkable();

    static constexpr int WINDOW_WIDTH = 1280;
    static constexpr int WINDOW_HEIGHT = 720;
    static constexpr int GRID_WIDTH = 40;
    static constexpr int GRID_HEIGHT = 25;
    static constexpr float CELL_SIZE = 24.0f;

    Grid grid_{GRID_WIDTH, GRID_HEIGHT};
    CoordinateSystem coords_{CELL_SIZE, {40.0f, 40.0f}};
    Renderer renderer_;
    AlgorithmAnimator animator_;
    UIOverlay overlay_;
    PathResult lastResult_;
    bool resultCaptured_ = false;

    BFS bfs_;
    Dijkstra dijkstra_;
    AStar aStar_;
    std::vector<IPathfinder*> algorithms_;
    int currentAlgorithmIndex_ = 0;
    IPathfinder* currentPathfinder_ = &bfs_;

    int currentHeuristicIndex_ = 0;
    bool use8Dir_ = false;

    // Comparison mode
    bool compareMode_ = false;
    int compAlgorithmIndex_ = -1;
    IPathfinder* compPathfinder_ = nullptr;
    AlgorithmAnimator compAnimator_;
    PathResult compResult_;
    bool compResultCaptured_ = false;

    TerrainType currentBrush_ = TerrainType::Wall;

    Vec2i start_{1, 1};
    Vec2i goal_{GRID_WIDTH - 2, GRID_HEIGHT - 2};

    MapEditor mapEditor_;
    BenchmarkRunner benchRunner_;
    ScenarioManager scenarioMgr_;
    MapMetadata mapMeta_;
};
