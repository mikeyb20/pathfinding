#include "App.h"
#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"

void App::init() {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pathfinding Visualizer");
    SetTargetFPS(60);
    rlImGuiSetup(true);

    algorithms_ = {&bfs_, &dijkstra_, &aStar_};
    currentAlgorithmIndex_ = 0;
    currentPathfinder_ = algorithms_[0];

    animator_.setPathfinder(currentPathfinder_);
    overlay_.init();
    scenarioMgr_.init();
    mapMeta_.start = start_;
    mapMeta_.goal = goal_;
    generateTestMap();
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

void App::switchAlgorithm(int index) {
    if (index < 0 || index >= static_cast<int>(algorithms_.size())) return;
    animator_.reset();
    currentAlgorithmIndex_ = index;
    currentPathfinder_ = algorithms_[index];
    animator_.setPathfinder(currentPathfinder_);
    lastResult_ = PathResult{};
    resultCaptured_ = false;
}

void App::handleInput() {
    // Algorithm switch from UI
    int requested = overlay_.getRequestedAlgorithm();
    if (requested >= 0) {
        switchAlgorithm(requested);
        overlay_.clearAlgorithmRequest();
    }

    // Heuristic switch from UI
    int reqHeuristic = overlay_.getRequestedHeuristic();
    if (reqHeuristic >= 0) {
        aStar_.setHeuristicByIndex(reqHeuristic);
        currentHeuristicIndex_ = aStar_.getHeuristicIndex();
        animator_.reset();
        lastResult_ = PathResult{};
        resultCaptured_ = false;
        overlay_.clearHeuristicRequest();
    }

    // 8-directional toggle from UI
    if (overlay_.getDiagToggled()) {
        use8Dir_ = !use8Dir_;
        for (auto* algo : algorithms_) {
            algo->setDiagonalMovement(use8Dir_);
        }
        if (compPathfinder_) {
            compPathfinder_->setDiagonalMovement(use8Dir_);
        }
        animator_.reset();
        compAnimator_.reset();
        lastResult_ = PathResult{};
        compResult_ = PathResult{};
        resultCaptured_ = false;
        compResultCaptured_ = false;
        overlay_.clearDiagToggle();
    }

    // Compare mode toggle from UI
    if (overlay_.getCompareModeChanged()) {
        compareMode_ = overlay_.getCompareMode();
        if (!compareMode_) {
            compAnimator_.reset();
            compPathfinder_ = nullptr;
            compAlgorithmIndex_ = -1;
            compResult_ = PathResult{};
            compResultCaptured_ = false;
        }
        overlay_.clearCompareModeChanged();
    }

    // Comparison algorithm switch from UI
    int reqComp = overlay_.getRequestedCompAlgorithm();
    if (reqComp >= 0) {
        compAlgorithmIndex_ = reqComp;
        compPathfinder_ = algorithms_[reqComp];
        compAnimator_.reset();
        compAnimator_.setPathfinder(compPathfinder_);
        compResult_ = PathResult{};
        compResultCaptured_ = false;
        overlay_.clearCompAlgorithmRequest();
    }

    bool imguiMouse = ImGui::GetIO().WantCaptureMouse;
    bool imguiKeyboard = ImGui::GetIO().WantCaptureKeyboard;

    // Terrain brush keybinds
    if (!imguiKeyboard) {
        if (IsKeyPressed(KEY_ONE))   currentBrush_ = TerrainType::Wall;
        if (IsKeyPressed(KEY_TWO))   currentBrush_ = TerrainType::Water;
        if (IsKeyPressed(KEY_THREE)) currentBrush_ = TerrainType::Mud;
        if (IsKeyPressed(KEY_FOUR))  currentBrush_ = TerrainType::Forest;
        if (IsKeyPressed(KEY_FIVE))  currentBrush_ = TerrainType::Open;
    }

    if (!imguiMouse) {
        // S + left click: set start position
        if (IsKeyDown(KEY_S) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vec2i cell = coords_.screenToGrid(
                static_cast<float>(GetMouseX()),
                static_cast<float>(GetMouseY())
            );
            if (grid_.inBounds(cell.x, cell.y) && grid_.isWalkable(cell.x, cell.y) && cell != goal_) {
                start_ = cell;
            }
        }
        // G + left click: set goal position
        else if (IsKeyDown(KEY_G) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vec2i cell = coords_.screenToGrid(
                static_cast<float>(GetMouseX()),
                static_cast<float>(GetMouseY())
            );
            if (grid_.inBounds(cell.x, cell.y) && grid_.isWalkable(cell.x, cell.y) && cell != start_) {
                goal_ = cell;
            }
        }
        // Left click: paint terrain with current brush
        else if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vec2i cell = coords_.screenToGrid(
                static_cast<float>(GetMouseX()),
                static_cast<float>(GetMouseY())
            );
            if (grid_.inBounds(cell.x, cell.y) && cell != start_ && cell != goal_) {
                grid_.setTerrain(cell.x, cell.y, currentBrush_);
            }
        }

        // Right click: erase to open
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vec2i cell = coords_.screenToGrid(
                static_cast<float>(GetMouseX()),
                static_cast<float>(GetMouseY())
            );
            if (grid_.inBounds(cell.x, cell.y)) {
                grid_.setTerrain(cell.x, cell.y, TerrainType::Open);
            }
        }
    }

    if (!imguiKeyboard) {
        // Space: run animated pathfinder
        if (IsKeyPressed(KEY_SPACE)) {
            animator_.start(grid_, start_, goal_);
            resultCaptured_ = false;
            if (compareMode_ && compPathfinder_) {
                compAnimator_.start(grid_, start_, goal_);
                compResultCaptured_ = false;
            }
        }

        // Enter: instant solve (no animation)
        if (IsKeyPressed(KEY_ENTER)) {
            animator_.reset();
            lastResult_ = currentPathfinder_->findPath(grid_, start_, goal_);
            resultCaptured_ = true;
            if (compareMode_ && compPathfinder_) {
                compAnimator_.reset();
                compResult_ = compPathfinder_->findPath(grid_, start_, goal_);
                compResultCaptured_ = true;
            }
        }

        // R: reset
        if (IsKeyPressed(KEY_R)) {
            animator_.reset();
            compAnimator_.reset();
            grid_.clear();
            lastResult_ = PathResult{};
            compResult_ = PathResult{};
            resultCaptured_ = false;
            compResultCaptured_ = false;
        }

        // P: pause/unpause
        if (IsKeyPressed(KEY_P)) {
            animator_.setPaused(!animator_.isPaused());
        }
    }

    // Handle UIOverlay reset button
    if (overlay_.resetRequested()) {
        animator_.reset();
        compAnimator_.reset();
        grid_.clear();
        lastResult_ = PathResult{};
        compResult_ = PathResult{};
        resultCaptured_ = false;
        compResultCaptured_ = false;
        overlay_.clearResetRequest();
    }

    // Handle map changed (load/generate)
    if (overlay_.mapChanged()) {
        animator_.reset();
        compAnimator_.reset();
        start_ = mapMeta_.start;
        goal_ = mapMeta_.goal;
        ensureStartGoalWalkable();
        lastResult_ = PathResult{};
        compResult_ = PathResult{};
        resultCaptured_ = false;
        compResultCaptured_ = false;
        overlay_.clearMapChanged();
    }

    // Handle benchmark request
    if (overlay_.benchmarkRequested()) {
        benchRunner_.run(grid_, start_, goal_, algorithms_);
        overlay_.clearBenchmarkRequest();
    }

    // Keep mapMeta in sync with start/goal
    mapMeta_.start = start_;
    mapMeta_.goal = goal_;
}

void App::update() {
    float dt = GetFrameTime();
    animator_.update(dt);

    // Capture result when animation finishes
    if (animator_.isFinished() && !resultCaptured_) {
        const auto& state = animator_.getState();
        if (state.finished) {
            lastResult_.path = state.currentPath;
            lastResult_.nodesExpanded = static_cast<int>(state.visitedOrder.size());
            if (state.pathFound && state.currentPath.size() > 1) {
                float cost = 0.0f;
                for (size_t i = 1; i < state.currentPath.size(); ++i) {
                    const auto& p = state.currentPath[i];
                    cost += grid_.getCell(p.x, p.y).movementCost;
                }
                lastResult_.totalCost = cost;
            } else {
                lastResult_.totalCost = 0.0f;
            }
            lastResult_.visitedOrder = state.visitedOrder;
            resultCaptured_ = true;
        }
    }

    // Update comparison animator
    if (compareMode_ && compPathfinder_) {
        compAnimator_.update(dt);

        if (compAnimator_.isFinished() && !compResultCaptured_) {
            const auto& cstate = compAnimator_.getState();
            if (cstate.finished) {
                compResult_.path = cstate.currentPath;
                compResult_.nodesExpanded = static_cast<int>(cstate.visitedOrder.size());
                if (cstate.pathFound && cstate.currentPath.size() > 1) {
                    float cost = 0.0f;
                    for (size_t i = 1; i < cstate.currentPath.size(); ++i) {
                        const auto& p = cstate.currentPath[i];
                        cost += grid_.getCell(p.x, p.y).movementCost;
                    }
                    compResult_.totalCost = cost;
                } else {
                    compResult_.totalCost = 0.0f;
                }
                compResult_.visitedOrder = cstate.visitedOrder;
                compResultCaptured_ = true;
            }
        }
    }
}

void App::draw() {
    BeginDrawing();
    ClearBackground({30, 30, 30, 255});

    renderer_.drawGrid(grid_, coords_);
    renderer_.drawSearchOverlay(animator_.getState(), coords_);

    // Comparison overlay
    if (compareMode_ && compPathfinder_) {
        renderer_.drawCompSearchOverlay(compAnimator_.getState(), coords_);
    }

    renderer_.drawStartGoal(start_, goal_, coords_);

    // Show path from animated search
    if (animator_.isFinished() && animator_.getState().pathFound) {
        renderer_.drawPath(animator_.getState().currentPath, coords_);
    }
    else if (!animator_.isRunning() && resultCaptured_ && lastResult_.found()) {
        renderer_.drawPath(lastResult_.path, coords_);
    }

    // Show comparison path
    if (compareMode_ && compPathfinder_) {
        if (compAnimator_.isFinished() && compAnimator_.getState().pathFound) {
            renderer_.drawCompPath(compAnimator_.getState().currentPath, coords_);
        } else if (!compAnimator_.isRunning() && compResultCaptured_ && compResult_.found()) {
            renderer_.drawCompPath(compResult_.path, coords_);
        }
    }

    // ImGui overlay
    static SearchState emptyState;
    const SearchState& compState = (compareMode_ && compPathfinder_)
        ? compAnimator_.getState() : emptyState;
    bool compFinished = (compareMode_ && compPathfinder_) && compAnimator_.isFinished();

    rlImGuiBegin();
    overlay_.draw(
        currentPathfinder_->getName(),
        animator_.getState(),
        lastResult_,
        animator_,
        algorithms_,
        currentAlgorithmIndex_,
        currentBrush_,
        currentHeuristicIndex_,
        use8Dir_,
        compareMode_,
        compAlgorithmIndex_,
        compResult_,
        compState,
        compFinished,
        mapEditor_,
        mapMeta_,
        benchRunner_,
        scenarioMgr_,
        grid_,
        start_,
        goal_
    );
    rlImGuiEnd();

    EndDrawing();
}

void App::generateTestMap() {
    auto set = [&](int x, int y, TerrainType t) {
        if (grid_.inBounds(x, y)) grid_.setTerrain(x, y, t);
    };

    // 1. Forest region (NW) — organic teardrop around start
    //    Row-by-row irregular shape
    struct Span { int y; int x0; int x1; };
    Span nwForest[] = {
        {0, 5, 7},
        {1, 4, 7},
        {2, 4, 8},
        {3, 3, 8},
        {4, 3, 7},
        {5, 4, 6},
        {6, 5, 5},
    };
    for (auto& s : nwForest)
        for (int x = s.x0; x <= s.x1; ++x) set(x, s.y, TerrainType::Forest);

    // Dense core — impassable undergrowth inside the forest
    Span nwForestCore[] = {
        {1, 5, 6},
        {2, 5, 7},
        {3, 4, 7},
        {4, 4, 6},
        {5, 5, 5},
    };
    for (auto& s : nwForestCore)
        for (int x = s.x0; x <= s.x1; ++x) set(x, s.y, TerrainType::Wall);

    // 2. Winding ridge/wall — diagonal cliff from (17,2) curving to (19,11)
    //    Each row has one or two wall cells, shifting to create a natural curve
    int ridgeX[] = { 17, 17, 17, 17, 17, 17, 18, 18, 19, 19 };
    for (int i = 0; i < 10; ++i) {
        set(ridgeX[i], 2 + i, TerrainType::Wall);
        // Thicken parts of the ridge
        if (i >= 0 && i <= 5) set(ridgeX[i] + 1, 2 + i, TerrainType::Wall);
    }
    // Extra bit at top for thickness
    set(19, 2, TerrainType::Wall);

    // 3. Lake (NE) — organic water body, wider in middle, tapering at edges
    Span lake[] = {
        {3, 24, 26},
        {4, 23, 27},
        {5, 22, 28},
        {6, 22, 28},
        {7, 23, 27},
        {8, 24, 26},
    };
    for (auto& s : lake)
        for (int x = s.x0; x <= s.x1; ++x) set(x, s.y, TerrainType::Water);

    // 4. Swamp/mud (W) — irregular wetland near the forest
    Span swamp[] = {
        {9,  2, 4},
        {10, 1, 4},
        {11, 2, 5},
        {12, 3, 5},
        {13, 4, 4},
    };
    for (auto& s : swamp)
        for (int x = s.x0; x <= s.x1; ++x) set(x, s.y, TerrainType::Mud);

    // 5. Large forest (E) — dense woodland blob
    Span eastForest[] = {
        {8,  30, 35},
        {9,  29, 35},
        {10, 28, 35},
        {11, 27, 35},
        {12, 29, 35},
        {13, 33, 35},
    };
    for (auto& s : eastForest)
        for (int x = s.x0; x <= s.x1; ++x) set(x, s.y, TerrainType::Forest);

    // 6. Ruined walls (center-bottom) — ring with opening on east side
    Span ruins[] = {
        {15, 9, 11},
        {16, 8, 9},  {16, 11, 12},
        {17, 8, 8},  {17, 12, 12},
        {18, 8, 9},  {18, 11, 12},
        {19, 9, 11},
    };
    for (auto& s : ruins)
        for (int x = s.x0; x <= s.x1; ++x) set(x, s.y, TerrainType::Wall);

    // 7. Mud bog (S-center) — natural swamp area
    Span bog[] = {
        {17, 19, 21},
        {18, 18, 22},
        {19, 18, 22},
        {20, 19, 21},
    };
    for (auto& s : bog)
        for (int x = s.x0; x <= s.x1; ++x) set(x, s.y, TerrainType::Mud);

    // 8. Rock formation (SE) — boulder cluster, irregular
    Span rocks[] = {
        {15, 31, 32},
        {16, 30, 33},
        {17, 31, 32}, {17, 30, 30},
        {18, 31, 32},
    };
    for (auto& s : rocks)
        for (int x = s.x0; x <= s.x1; ++x) set(x, s.y, TerrainType::Wall);

    // 9. Small pond (SW-bottom) — water feature near goal path
    Span pond[] = {
        {22, 11, 13},
        {23, 10, 14},
        {24, 11, 13},
    };
    for (auto& s : pond)
        for (int x = s.x0; x <= s.x1; ++x) set(x, s.y, TerrainType::Water);
}

void App::ensureStartGoalWalkable() {
    // Clamp to bounds
    if (!grid_.inBounds(start_.x, start_.y)) {
        start_ = {1, 1};
    }
    if (!grid_.inBounds(goal_.x, goal_.y)) {
        goal_ = {grid_.getWidth() - 2, grid_.getHeight() - 2};
    }
    // Force start/goal cells to Open if they became impassable
    if (!grid_.isWalkable(start_.x, start_.y)) {
        grid_.setTerrain(start_.x, start_.y, TerrainType::Open);
    }
    if (!grid_.isWalkable(goal_.x, goal_.y)) {
        grid_.setTerrain(goal_.x, goal_.y, TerrainType::Open);
    }
}
