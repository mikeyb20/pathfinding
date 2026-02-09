cmake_minimum_required(VERSION 3.20)
project(pathfinder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Dependencies via FetchContent ---
include(FetchContent)

# raylib
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raylib)

# Dear ImGui (no CMakeLists — we build it manually)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.5
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

# rlImGui (no CMakeLists — we build it manually)
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(rlimgui)

add_library(rlimgui STATIC
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)
target_include_directories(rlimgui PUBLIC ${rlimgui_SOURCE_DIR})
target_link_libraries(rlimgui PUBLIC imgui raylib)

# nlohmann/json (release tarball for fast download)
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

# nanobench
FetchContent_Declare(
    nanobench
    GIT_REPOSITORY https://github.com/martinus/nanobench.git
    GIT_TAG        v4.3.11
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nanobench)

# FastNoiseLite (header-only)
FetchContent_Declare(
    fastnoiselite
    GIT_REPOSITORY https://github.com/Auburn/FastNoiseLite.git
    GIT_TAG        v1.1.1
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(fastnoiselite)

add_library(fastnoiselite INTERFACE)
target_include_directories(fastnoiselite INTERFACE ${fastnoiselite_SOURCE_DIR}/Cpp)

# --- Main executable ---
add_executable(pathfinder
    src/main.cpp
    src/App.cpp
    src/Core/Grid.cpp
    src/Core/CoordinateSystem.cpp
    src/Algorithms/BFS.cpp
    src/Algorithms/Dijkstra.cpp
    src/Algorithms/AStar.cpp
    src/Algorithms/JumpPointSearch.cpp
    src/Visualization/Renderer.cpp
    src/Visualization/AlgorithmAnimator.cpp
    src/Visualization/UIOverlay.cpp
    src/MapEditor/MapEditor.cpp
    src/Benchmarking/BenchmarkRunner.cpp
    src/Scenarios/ScenarioManager.cpp
)

target_include_directories(pathfinder PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(pathfinder PRIVATE
    raylib
    imgui
    rlimgui
    nlohmann_json::nlohmann_json
    nanobench
    fastnoiselite
)

target_compile_options(pathfinder PRIVATE
    -Wall -Wextra -Wpedantic
)
