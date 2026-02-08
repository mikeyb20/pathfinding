#include "MapEditor/MapEditor.h"
#include "imgui.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <random>
#include "FastNoiseLite.h"

namespace fs = std::filesystem;

std::string MapEditor::mapsDirectory() const {
    std::string dir = "data/maps/";
    fs::create_directories(dir);
    return dir;
}

bool MapEditor::saveMap(const Grid& grid, const MapMetadata& meta, const std::string& filename) {
    nlohmann::json j;
    j["version"] = 1;
    j["name"] = meta.name;
    j["description"] = meta.description;
    j["start"] = {{"x", meta.start.x}, {"y", meta.start.y}};
    j["goal"] = {{"x", meta.goal.x}, {"y", meta.goal.y}};
    j["grid"] = grid.toJson();

    std::string path = mapsDirectory() + filename + ".json";
    std::ofstream out(path);
    if (!out.is_open()) return false;
    out << j.dump(2);
    return out.good();
}

bool MapEditor::loadMap(Grid& grid, MapMetadata& meta, const std::string& filename) {
    std::string path = mapsDirectory() + filename + ".json";
    std::ifstream in(path);
    if (!in.is_open()) return false;

    nlohmann::json j;
    try {
        in >> j;
    } catch (...) {
        return false;
    }

    if (j.contains("name")) meta.name = j["name"].get<std::string>();
    if (j.contains("description")) meta.description = j["description"].get<std::string>();
    if (j.contains("start")) {
        meta.start.x = j["start"]["x"].get<int>();
        meta.start.y = j["start"]["y"].get<int>();
    }
    if (j.contains("goal")) {
        meta.goal.x = j["goal"]["x"].get<int>();
        meta.goal.y = j["goal"]["y"].get<int>();
    }
    if (j.contains("grid")) {
        grid.fromJson(j["grid"]);
    }
    return true;
}

std::vector<std::string> MapEditor::listSavedMaps() const {
    std::vector<std::string> maps;
    std::string dir = "data/maps/";
    if (!fs::exists(dir)) return maps;

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (entry.path().extension() == ".json") {
            maps.push_back(entry.path().stem().string());
        }
    }
    std::sort(maps.begin(), maps.end());
    return maps;
}

void MapEditor::generateNoise(Grid& grid, int seed, float frequency, int octaves) {
    FastNoiseLite noise;
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetSeed(seed);
    noise.SetFrequency(frequency);
    noise.SetFractalOctaves(octaves);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);

    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            float val = noise.GetNoise(static_cast<float>(x), static_cast<float>(y));
            TerrainType t;
            if (val < -0.4f)      t = TerrainType::Water;
            else if (val < -0.1f) t = TerrainType::Mud;
            else if (val < 0.2f)  t = TerrainType::Open;
            else if (val < 0.5f)  t = TerrainType::Forest;
            else                  t = TerrainType::Wall;
            grid.setTerrain(x, y, t);
        }
    }
}

void MapEditor::generateRandom(Grid& grid, float density, int seed) {
    std::mt19937 rng(static_cast<unsigned>(seed));
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int y = 0; y < grid.getHeight(); ++y) {
        for (int x = 0; x < grid.getWidth(); ++x) {
            if (dist(rng) < density)
                grid.setTerrain(x, y, TerrainType::Wall);
            else
                grid.setTerrain(x, y, TerrainType::Open);
        }
    }
}

bool MapEditor::drawUI(Grid& grid, MapMetadata& meta) {
    bool gridChanged = false;

    // --- Save/Load Section ---
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Save / Load");
    ImGui::Separator();

    ImGui::InputText("Filename", filenameBuf_, sizeof(filenameBuf_));
    ImGui::InputText("Map Name", mapNameBuf_, sizeof(mapNameBuf_));
    ImGui::InputTextMultiline("Desc", descBuf_, sizeof(descBuf_),
                              ImVec2(-1, ImGui::GetTextLineHeight() * 3));

    if (ImGui::Button("Save Map")) {
        meta.name = mapNameBuf_;
        meta.description = descBuf_;
        if (saveMap(grid, meta, filenameBuf_)) {
            statusMessage_ = "Saved: " + std::string(filenameBuf_) + ".json";
            needsRefresh_ = true;
        } else {
            statusMessage_ = "Save failed!";
        }
    }

    ImGui::Separator();

    // Refresh cached list
    if (needsRefresh_) {
        cachedMapList_ = listSavedMaps();
        needsRefresh_ = false;
    }

    if (!cachedMapList_.empty()) {
        if (ImGui::BeginListBox("Maps", ImVec2(-1, ImGui::GetTextLineHeightWithSpacing() * 5))) {
            for (int i = 0; i < static_cast<int>(cachedMapList_.size()); ++i) {
                bool selected = (i == selectedMapIndex_);
                if (ImGui::Selectable(cachedMapList_[i].c_str(), selected)) {
                    selectedMapIndex_ = i;
                }
            }
            ImGui::EndListBox();
        }

        if (ImGui::Button("Load Map") && selectedMapIndex_ >= 0
            && selectedMapIndex_ < static_cast<int>(cachedMapList_.size())) {
            if (loadMap(grid, meta, cachedMapList_[selectedMapIndex_])) {
                snprintf(filenameBuf_, sizeof(filenameBuf_), "%s",
                         cachedMapList_[selectedMapIndex_].c_str());
                snprintf(mapNameBuf_, sizeof(mapNameBuf_), "%s", meta.name.c_str());
                snprintf(descBuf_, sizeof(descBuf_), "%s", meta.description.c_str());
                statusMessage_ = "Loaded: " + cachedMapList_[selectedMapIndex_];
                gridChanged = true;
            } else {
                statusMessage_ = "Load failed!";
            }
        }
    } else {
        ImGui::TextDisabled("No saved maps found");
    }

    if (!statusMessage_.empty()) {
        ImGui::TextWrapped("%s", statusMessage_.c_str());
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // --- Generation Section ---
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "Procedural Generation");
    ImGui::Separator();

    ImGui::Text("Noise Map");
    ImGui::InputInt("Noise Seed", &noiseSeed_);
    ImGui::SliderFloat("Frequency", &noiseFrequency_, 0.01f, 0.3f, "%.3f");
    ImGui::SliderInt("Octaves", &noiseOctaves_, 1, 6);
    if (ImGui::Button("Generate Noise")) {
        generateNoise(grid, noiseSeed_, noiseFrequency_, noiseOctaves_);
        gridChanged = true;
    }

    ImGui::Spacing();
    ImGui::Text("Random Walls");
    ImGui::SliderFloat("Density", &randomDensity_, 0.0f, 0.6f, "%.2f");
    ImGui::InputInt("Random Seed", &randomSeed_);
    if (ImGui::Button("Generate Random")) {
        generateRandom(grid, randomDensity_, randomSeed_);
        gridChanged = true;
    }

    return gridChanged;
}
