//
// Created by efn on 4/27/26.
//

#ifndef UNTITLED_TURTLE_H
#define UNTITLED_TURTLE_H

// Turtle.hpp

#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/server.hpp>
#include <vector>
#include <string>


#include "Utils.h"
using namespace std;

struct JourneyStep {
    Vec3 position;
    std::optional<frbludDirections> frbludDirection;
    long long arriveTime{};
    std::optional<long long> leaveTime;
    long long waitTime{};

    friend void to_json(nlohmann::json& j, const JourneyStep& s) {
        j = nlohmann::json{
            {"position", s.position},
            {"frbludDirection", s.frbludDirection},
            {"arriveTime", s.arriveTime},
            {"waitTime", s.waitTime}
        };
    }

    friend void from_json(const nlohmann::json& j, JourneyStep& s) {
        s.position = j.at("position").get<Vec3>();
        s.frbludDirection = stringToFrblud.at(j.at("frbludDirection").get<string>());
        s.arriveTime = j.at("arriveTime").get<long long>();

        if (j.contains("leaveTime") && !j.at("leaveTime").is_null()) {
            s.leaveTime = j.at("leaveTime").get<long long>();
        } else {
            s.leaveTime = std::nullopt;
        }

        s.waitTime = j.at("waitTime").get<long long>();
    }
};

struct TurtleData {
    int id;
    Vec3 position;
    string dimension;
    int fuel;
    bool busy;
    neswDirections face;
    vector<JourneyStep> journeyPath;
    int journeyStepIndex;

    friend void from_json(const nlohmann::json& j, TurtleData& t) {
        t.id = j.at("id").get<int>();
        t.position = j.at("position").get<Vec3>();
        t.dimension = j.at("dimension").get<string>();
        t.fuel = j.at("fuel").get<int>();
        t.busy = j.at("busy").get<bool>();
        t.face = j.at("face").get<neswDirections>();
        t.journeyPath = j.at("journeyPath").get<vector<JourneyStep>>();
        t.journeyStepIndex = j.at("journeyStepIndex").get<int>();
    }
};

struct Turtle {
    int id;
    websocketpp::connection_hdl ws;
    Vec3 position;
    string dimension;
    int fuel;
    bool busy;
    neswDirections face;
    vector<JourneyStep> journeyPath;
    int journeyStepIndex;

    Turtle(websocketpp::connection_hdl ws_, const TurtleData& data);
    void Update(const TurtleData& data);
};

#endif //UNTITLED_TURTLE_H
