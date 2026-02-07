#pragma once

#include <string>
#include "Core/SearchState.h"
#include "Core/PathResult.h"

class UIOverlay {
public:
    void init();
    void shutdown();
    void draw(const std::string& algorithmName, const SearchState& state, const PathResult& lastResult);
};
