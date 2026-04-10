// WorldMap.hpp
#pragma once
#include <unordered_map>
#include "Utils.hpp"
#include <vector>
#include <string>
#include <chrono>
#include <optional>

#include <Turtle.hpp>

using namespace std;

struct Block {
    bool blocked;
    string name;
    unordered_map<string, string> state;
    vector<string> tags;

    optional<long long> placeTime;
    optional<long long> removeTime;
    
    friend void from_json(const nlohmann::json& j, Block& b) {
        b.blocked = j.at("blocked").get<bool>();
        b.name = j.at("name").get<string>();
        b.state = j.at("state").get<unordered_map<string, string>>();
        b.tags = j.at("tags").get<vector<string>>();
        
        if (j.contains("placeTime") && !j.at("placeTime").is_null()) {
            b.placeTime = j.at("placeTime").get<long long>();
        } else {
            b.placeTime = std::nullopt;
        }

        if (j.contains("removeTime") && !j.at("removeTime").is_null()) {
            b.removeTime = j.at("removeTime").get<long long>();
        } else {
            b.removeTime = std::nullopt;
        }
    }
    
    friend void to_json(nlohmann::json& j, const Block& b) {
        j = nlohmann::json{
            {"blocked", b.blocked},
            {"name", b.name},
            {"state", b.state},
            {"tags", b.tags},
            {"placeTime", b.placeTime},
            {"removeTime", b.removeTime}
        };
    }
};

struct TurtleReservation {
    int turtleId;
    long long arriveTime;
    optional<long long> leaveTime;

    friend void to_json(nlohmann::json& j, const TurtleReservation& r) {
        j = nlohmann::json{
            {"turtleId", r.turtleId},
            {"arriveTime", r.arriveTime},
            {"leaveTime", r.leaveTime}
        };
    }

    friend void from_json(const nlohmann::json& j, TurtleReservation& r) {
        r.turtleId = j.at("turtleId").get<int>();
        r.arriveTime = j.at("arriveTime").get<long long>();
        if (j.contains("leaveTime") && !j.at("leaveTime").is_null()) {
            r.leaveTime = j.at("leaveTime").get<long long>();
        } else {
            r.leaveTime = std::nullopt;
        }
    }
};

struct MapEntry {
    vector<Block> blocks; // List of blocks at this position
    vector<TurtleReservation> reservations; // List of turtle reservations for this position
    
    friend void from_json(const nlohmann::json& j, MapEntry& e) {
        e.blocks = j.at("blocks").get<vector<Block>>();
        e.reservations = j.at("reservations").get<vector<TurtleReservation>>();
    }
    
    friend void to_json(nlohmann::json& j, const MapEntry& e) {
        j = nlohmann::json{
            {"blocks", e.blocks},
            {"reservations", e.reservations}
        };
    }
};


class WorldMap {
    private:
        string mapFilePath;
        void CleanEntry(Vec3 pos);
        unordered_map<Vec3, MapEntry> map;
    public:
        WorldMap(string mapFilePath);
        MapEntry Get(Vec3 pos);
        void SonarUpdate(Vec3 pos, Block block);
        void Save();
        bool MakeReservation(int id, Vec3 position, long long arriveTime, long long leaveTime);
};