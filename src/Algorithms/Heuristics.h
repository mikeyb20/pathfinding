#pragma once

#include <cmath>
#include <algorithm>
#include "Core/Types.h"

namespace Heuristics {

inline float manhattan(Vec2i a, Vec2i b) {
    return static_cast<float>(std::abs(a.x - b.x) + std::abs(a.y - b.y));
}

inline float euclidean(Vec2i a, Vec2i b) {
    float dx = static_cast<float>(a.x - b.x);
    float dy = static_cast<float>(a.y - b.y);
    return std::sqrt(dx * dx + dy * dy);
}

inline float octile(Vec2i a, Vec2i b) {
    float dx = static_cast<float>(std::abs(a.x - b.x));
    float dy = static_cast<float>(std::abs(a.y - b.y));
    static const float SQRT2_MINUS_1 = std::sqrt(2.0f) - 1.0f;
    return std::max(dx, dy) + SQRT2_MINUS_1 * std::min(dx, dy);
}

inline float chebyshev(Vec2i a, Vec2i b) {
    return static_cast<float>(std::max(std::abs(a.x - b.x), std::abs(a.y - b.y)));
}

} // namespace Heuristics
