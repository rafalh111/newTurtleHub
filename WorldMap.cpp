// WorldMap.cpp
#include <nlohmann/json.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>

#include "WorldMap.hpp"

using namespace std;
using json = nlohmann::json;
namespace fs = filesystem;

WorldMap::WorldMap(string mapFilePath_) : mapFilePath(mapFilePath_) {
    if (!fs::exists(mapFilePath)) {
        cout << "Map file does not exist, starting with an empty map: " << mapFilePath << endl;
        return;
    }

    try {
        ifstream inputFile(mapFilePath);
        if (!inputFile.is_open()) {
            throw runtime_error("Failed to open map file: " + mapFilePath);
        }

        json j;
        inputFile >> j;

        for (auto& [stringKey, entryJson] : j.items()) {
            Vec3 pos = Vec3::fromString(stringKey);
            MapEntry entry = entryJson.get<MapEntry>();
            map[pos] = entry;
        }

        inputFile.close();
    } catch (const exception& e) {
        cerr << "Error loading map from file: " << e.what() << endl;
    }
}

void WorldMap::CleanEntry(Vec3 pos) {
    auto& entry = map[pos];

    long long now = chrono::duration_cast<chrono::milliseconds>(
        chrono::system_clock::now().time_since_epoch()
    ).count();

    for (int i = entry.blocks.size() - 1; i >= 0; --i) {
        if (entry.blocks[i].removeTime.has_value() && entry.blocks[i].removeTime.value() < now) {
            entry.blocks.erase(entry.blocks.begin() + i);
        }
    }

    for (int i = entry.reservations.size() - 1; i >= 0; --i) {
        if (entry.reservations[i].leaveTime.has_value() && entry.reservations[i].leaveTime.value() < now) {
            entry.reservations.erase(entry.reservations.begin() + i);
        }
    }

    if (entry.blocks.empty() && entry.reservations.empty()) {
        map.erase(pos);
    }
};

MapEntry WorldMap::Get(Vec3 pos) {
    CleanEntry(pos);
    return map[pos];
}

void WorldMap::SonarUpdate(Vec3 pos, Block block) {
    if (!map.count(pos)) {
        MapEntry newEntry;
        newEntry.blocks.push_back(block);
        map[pos] = MapEntry();

        map[pos].blocks.push_back(block);
    }

    MapEntry& entry = map[pos];
    
    for (int i = 0; i < entry.blocks.size(); ++i) {
        if (!(entry.blocks[i].placeTime >= block.placeTime && entry.blocks[i].removeTime <= block.removeTime)) {
            continue;
        }

        if (block.blocked == false) {
            entry.blocks.erase(entry.blocks.begin() + i);

            if (entry.blocks.empty() && entry.reservations.empty()) {
                map.erase(pos);
            }
            
            return;
        }

        return;
    }

    entry.blocks.push_back(block);
}

void WorldMap::Save() {
    try {
        nlohmann::json j;
        for (const auto& [pos, entry] : map) {
            j[pos.toString()] = entry;
        }

        ofstream outputFile(mapFilePath);
        if (!outputFile.is_open()) {
            throw runtime_error("Failed to open map file for saving: " + mapFilePath);
        }

        outputFile << j.dump(4) << endl;
        outputFile.close();
    } catch (const exception& e) {
        cerr << "Error saving map to file: " << e.what() << endl;
    }
}