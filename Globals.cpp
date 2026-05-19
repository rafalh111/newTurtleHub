//
// Created by efn on 5/4/26.
//

#include "Globals.h"
std::unordered_map<std::string, WorldMap*>& getDimensionMaps() {
    static std::unordered_map<std::string, WorldMap*> maps = {
        {"overworld", &OverworldMap},
        {"nether", &NetherMap},
        {"end", &EndMap}
    };
    return maps;
}