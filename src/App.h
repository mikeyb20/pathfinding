#pragma once

#include <memory>
#include "Core/Grid.h"
#include "Core/CoordinateSystem.h"
#include "Core/PathResult.h"
#include "Algorithms/IPathfinder.h"
#include "Algorithms/BFS.h"
#include "Visualization/Renderer.h"
#include "Visualization/AlgorithmAnimator.h"
#include "Visualization/UIOverlay.h"

class App {
public:
    void init();
    void run();
    void shutdown();

private:
    void handleInput();
    void update();
    void draw();

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

    BFS bfs_;
    IPathfinder* currentPathfinder_ = &bfs_;

    Vec2i start_{1, 1};
    Vec2i goal_{GRID_WIDTH - 2, GRID_HEIGHT - 2};
};
