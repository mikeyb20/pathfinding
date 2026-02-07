#include "App.h"
#include "raylib.h"
#include "rlImGui.h"

void App::init() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pathfinding Visualizer");
    SetTargetFPS(60);
    rlImGuiSetup(true);

    animator_.setPathfinder(currentPathfinder_);
    overlay_.init();
}

void App::run() {
    while (!WindowShouldClose()) {
        handleInput();
        update();
        draw();
    }
}

void App::shutdown() {
    overlay_.shutdown();
    rlImGuiShutdown();
    CloseWindow();
}

void App::handleInput() {
    // Left click: place walls
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vec2i cell = coords_.screenToGrid(
            static_cast<float>(GetMouseX()),
            static_cast<float>(GetMouseY())
        );
        if (grid_.inBounds(cell.x, cell.y) && cell != start_ && cell != goal_) {
            grid_.setTerrain(cell.x, cell.y, TerrainType::Wall);
        }
    }

    // Right click: remove walls
    if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        Vec2i cell = coords_.screenToGrid(
            static_cast<float>(GetMouseX()),
            static_cast<float>(GetMouseY())
        );
        if (grid_.inBounds(cell.x, cell.y)) {
            grid_.setTerrain(cell.x, cell.y, TerrainType::Open);
        }
    }

    // Space: run pathfinder
    if (IsKeyPressed(KEY_SPACE)) {
        animator_.start(grid_, start_, goal_);
    }

    // R: reset
    if (IsKeyPressed(KEY_R)) {
        animator_.reset();
        grid_.clear();
    }

    // P: pause/unpause
    if (IsKeyPressed(KEY_P)) {
        animator_.setPaused(!animator_.isPaused());
    }
}

void App::update() {
    animator_.update(GetFrameTime());
}

void App::draw() {
    BeginDrawing();
    ClearBackground({30, 30, 30, 255});

    renderer_.drawGrid(grid_, coords_);
    renderer_.drawSearchOverlay(animator_.getState(), coords_);
    renderer_.drawStartGoal(start_, goal_, coords_);

    if (animator_.isFinished() && animator_.getState().pathFound) {
        renderer_.drawPath(animator_.getState().currentPath, coords_);
    }

    // ImGui overlay
    rlImGuiBegin();
    overlay_.draw(
        currentPathfinder_->getName(),
        animator_.getState(),
        lastResult_
    );
    rlImGuiEnd();

    EndDrawing();
}
