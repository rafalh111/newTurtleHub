//
// Created by efn on 7/1/26.
//

module;
#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/frame.hpp>
#include <nlohmann/json.hpp>
#include <utility>
#include <vector>
#include <string>
#include <chrono>

export module Turtle;

import Utils;

// Forward declare and extern declare DimensionMaps which is defined in Globals
class WorldMap;
extern std::unordered_map<std::string, WorldMap*> DimensionMaps;

export struct TurtleData {
    int id;
    Vec3 position;
    WorldMap* dimension;
    int fuel;
    bool busy;
    neswDirections face;
    std::vector<JourneyStep> journeyPath;
    int journeyStepIndex;

    friend void from_json(const nlohmann::json& j, TurtleData& t) {
        t.id = j.at("id").get<int>();
        t.position = j.at("position").get<Vec3>();
        if (const auto it = DimensionMaps.find(j.at("dimension").get<std::string>()); it != DimensionMaps.end()) {
            t.dimension = it->second;
        } else {
            throw std::invalid_argument("Turtle with id: " + std::to_string(t.id) + " has a dimension that doesn't exist");
        }

        t.fuel = j.at("fuel").get<int>();
        t.busy = j.at("busy").get<bool>();
        t.face = j.at("face").get<neswDirections>();
        t.journeyPath = j.at("journeyPath").get<std::vector<JourneyStep>>();
        t.journeyStepIndex = j.at("journeyStepIndex").get<int>();
    }
};

export struct Turtle {
    int id;
    websocketpp::connection_hdl ws;
    Vec3 position;
    WorldMap* dimension;
    int fuel;
    bool busy;
    neswDirections face;
    std::vector<JourneyStep> journeyPath;
    int journeyStepIndex;

    Turtle(websocketpp::connection_hdl ws_, const TurtleData& data);
    void Update(const TurtleData& data);
    void ObstacleWarning(Vec3 vector);
};

Turtle::Turtle(websocketpp::connection_hdl ws_, const TurtleData& data) :
    id(data.id), ws(std::move(ws_)), position(data.position.x, data.position.y, data.position.z), dimension(data.dimension), fuel(data.fuel), busy(data.busy),
    face(data.face), journeyPath(data.journeyPath), journeyStepIndex(data.journeyStepIndex) {
}

void Turtle::Update(const TurtleData& data) {
    position = {data.position.x, data.position.y, data.position.z};
    dimension = data.dimension;
    fuel = data.fuel;
    busy = data.busy;
    face = data.face;
    journeyPath = data.journeyPath;
    journeyStepIndex = data.journeyStepIndex;
}

void Turtle::ObstacleWarning(const Vec3 vector) {
    // TODO: Send obstacle warning through websocket hub
    // This requires access to turtleHub which would create a circular dependency
    // Implementation should be moved to MessageHandlers where Globals is imported
}
