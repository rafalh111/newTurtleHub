//
// Created by efn on 7/1/26.
//

module;
#include <websocketpp/common/connection_hdl.hpp>
#include <nlohmann/json.hpp>
#include <optional>
#include <utility>
#include <vector>
#include <string>
#include <chrono>


export module Turtle;

import Utils;
// import ResponseMessages;
export enum class TurtleAction {
    Forward,
    Up,
    Down,
    TurnLeft,
    TurnRight
};

TurtleAction parseAction(const std::string& str) {
    if (str == "Forward") return TurtleAction::Forward;
    if (str == "Up")      return TurtleAction::Up;
    if (str == "Down")    return TurtleAction::Down;
    if (str == "TurnLeft")  return TurtleAction::TurnLeft;
    if (str == "TurnRight") return TurtleAction::TurnRight;
    throw std::invalid_argument("Unknown action");
}

export struct JourneyStep {
    Vec3 position;
    TurtleAction action;
    TimeInterval timeInterval;

    friend void to_json(nlohmann::json& j, const JourneyStep& s) {
        j = nlohmann::json{
        {"position", s.position},
        {"frbludDirection", s.action},
        {"timeInterval", s.timeInterval},
        };
    }

    friend void from_json(const nlohmann::json& j, JourneyStep& s) {
        s.position = j.at("position").get<Vec3>();
        s.action = parseAction(j.at("action").get<std::string>());
        s.timeInterval = j.at("timeInterval").get<TimeInterval>();
    }
};

export struct TurtleData {
    int id;
    Vec3 position;
    std::string dimension;
    int fuel;
    bool busy;
    neswDirections face;
    std::vector<JourneyStep> journeyPath;
    int journeyStepIndex;

    friend void from_json(const nlohmann::json& j, TurtleData& t) {
        t.id = j.at("id").get<int>();
        t.position = j.at("position").get<Vec3>();
        t.dimension = j.at("dimension").get<std::string>();
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
    std::string dimension;
    int fuel;
    bool busy;
    neswDirections face;
    std::vector<JourneyStep> journeyPath;
    int journeyStepIndex;

    Turtle(websocketpp::connection_hdl ws_, const TurtleData& data);
    void Update(const TurtleData& data);
    void ObstacleWarning(Vec3 vector);
}

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
    const nlohmann::json response = getObstacleWarningResponse(vector);
    turtleHub.send(ws, response.dump(), websocketpp::frame::opcode::text);
}
