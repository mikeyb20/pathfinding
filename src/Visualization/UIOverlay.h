#pragma once

#include <string>
#include <vector>
#include "Core/Types.h"
#include "Core/SearchState.h"
#include "Core/PathResult.h"
#include "Algorithms/IPathfinder.h"
#include "Visualization/AlgorithmAnimator.h"

class UIOverlay {
public:
    void init();
    void shutdown();
    void draw(const std::string& algorithmName, const SearchState& state,
              const PathResult& lastResult, AlgorithmAnimator& animator,
              const std::vector<IPathfinder*>& algorithms, int currentIndex,
              TerrainType currentBrush);

    bool resetRequested() const { return resetRequested_; }
    void clearResetRequest() { resetRequested_ = false; }

    int getRequestedAlgorithm() const { return requestedAlgorithm_; }
    void clearAlgorithmRequest() { requestedAlgorithm_ = -1; }

private:
    bool resetRequested_ = false;
    int requestedAlgorithm_ = -1;
};
