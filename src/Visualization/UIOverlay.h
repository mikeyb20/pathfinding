#pragma once

#include <string>
#include <vector>
#include "Core/Types.h"
#include "Core/SearchState.h"
#include "Core/PathResult.h"
#include "Algorithms/IPathfinder.h"
#include "Visualization/AlgorithmAnimator.h"
#include "MapEditor/MapEditor.h"
#include "Benchmarking/BenchmarkRunner.h"
#include "Scenarios/ScenarioManager.h"

class UIOverlay {
public:
    void init();
    void shutdown();
    void draw(const std::string& algorithmName, const SearchState& state,
              const PathResult& lastResult, AlgorithmAnimator& animator,
              const std::vector<IPathfinder*>& algorithms, int currentIndex,
              TerrainType currentBrush, int currentHeuristicIndex, bool use8Dir,
              bool compareMode, int compAlgorithmIndex,
              const PathResult& compResult, const SearchState& compState,
              bool compFinished,
              MapEditor& mapEditor, MapMetadata& mapMeta,
              BenchmarkRunner& benchRunner, ScenarioManager& scenarioMgr,
              Grid& grid, Vec2i& start, Vec2i& goal);

    bool resetRequested() const { return resetRequested_; }
    void clearResetRequest() { resetRequested_ = false; }

    int getRequestedAlgorithm() const { return requestedAlgorithm_; }
    void clearAlgorithmRequest() { requestedAlgorithm_ = -1; }

    int getRequestedHeuristic() const { return requestedHeuristic_; }
    void clearHeuristicRequest() { requestedHeuristic_ = -1; }

    bool getDiagToggled() const { return diagToggled_; }
    void clearDiagToggle() { diagToggled_ = false; }

    bool getCompareModeChanged() const { return compareModeChanged_; }
    void clearCompareModeChanged() { compareModeChanged_ = false; }
    bool getCompareMode() const { return compareModeValue_; }

    int getRequestedCompAlgorithm() const { return requestedCompAlgorithm_; }
    void clearCompAlgorithmRequest() { requestedCompAlgorithm_ = -1; }

    bool mapChanged() const { return mapChanged_; }
    void clearMapChanged() { mapChanged_ = false; }

    bool benchmarkRequested() const { return benchmarkRequested_; }
    void clearBenchmarkRequest() { benchmarkRequested_ = false; }

private:
    void drawPathfindingTab(const std::string& algorithmName, const SearchState& state,
                            const PathResult& lastResult, AlgorithmAnimator& animator,
                            const std::vector<IPathfinder*>& algorithms, int currentIndex,
                            TerrainType currentBrush, int currentHeuristicIndex, bool use8Dir,
                            bool compareMode, int compAlgorithmIndex,
                            const PathResult& compResult, const SearchState& compState,
                            bool compFinished);

    bool resetRequested_ = false;
    int requestedAlgorithm_ = -1;
    int requestedHeuristic_ = -1;
    bool diagToggled_ = false;
    bool compareModeChanged_ = false;
    bool compareModeValue_ = false;
    int requestedCompAlgorithm_ = -1;
    bool mapChanged_ = false;
    bool benchmarkRequested_ = false;
};
