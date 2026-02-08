#pragma once

#include <string>
#include <vector>
#include "Core/Grid.h"
#include "Core/Types.h"

struct MapMetadata {
    std::string name = "untitled";
    std::string description;
    Vec2i start{1, 1};
    Vec2i goal{38, 23};
};

class MapEditor {
public:
    bool saveMap(const Grid& grid, const MapMetadata& meta, const std::string& filename);
    bool loadMap(Grid& grid, MapMetadata& meta, const std::string& filename);
    std::vector<std::string> listSavedMaps() const;
    void generateNoise(Grid& grid, int seed, float frequency, int octaves);
    void generateRandom(Grid& grid, float density, int seed);
    bool drawUI(Grid& grid, MapMetadata& meta);  // returns true if grid modified

private:
    std::string mapsDirectory() const;

    char filenameBuf_[128] = "mymap";
    char mapNameBuf_[128] = "untitled";
    char descBuf_[256] = "";
    int selectedMapIndex_ = -1;
    std::vector<std::string> cachedMapList_;

    int noiseSeed_ = 42;
    float noiseFrequency_ = 0.08f;
    int noiseOctaves_ = 3;
    float randomDensity_ = 0.3f;
    int randomSeed_ = 42;

    std::string statusMessage_;
    bool needsRefresh_ = true;
};
