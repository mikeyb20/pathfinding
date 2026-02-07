#pragma once

#include <string>
#include "Core/Grid.h"
#include "Core/PathResult.h"
#include "Core/SearchState.h"

class IPathfinder {
public:
    virtual ~IPathfinder() = default;

    virtual PathResult findPath(const Grid& grid, Vec2i start, Vec2i goal) = 0;
    virtual std::string getName() const = 0;

    // Step-by-step visualization support
    virtual void initSearch(const Grid& grid, Vec2i start, Vec2i goal) = 0;
    virtual bool step() = 0; // returns false when done
    virtual const SearchState& getCurrentState() const = 0;
};
