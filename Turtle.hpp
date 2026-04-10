// Turtle.hpp
#pragma once

#include <websocketpp/server.hpp>
#include <vector>
#include <string>

#include "Utils.hpp"
using namespace std;

struct JourneyStep {
    Vec3 position;
    optional<frbludDirections> frbludDirection;
    optional<long long> arriveTime;
    optional<long long> leaveTime;
    long long waitTime;

    friend void to_json(nlohmann::json& j, const JourneyStep& s) {
        j = nlohmann::json{
            {"vector", s.position},
            {"frbludDirection", s.frbludDirection},
            {"arriveTime", s.arriveTime},
            {"waitTime", s.waitTime}
        };
    };

    friend void from_json(const nlohmann::json& j, JourneyStep& s) {
        s.position = j.at("position").get<Vec3>();
        s.frbludDirection = stringToFrblud.at(j.at("frbludDirection").get<string>());
        if (j.contains("arriveTime") && !j.at("arriveTime").is_null()) {
            s.arriveTime = j.at("arriveTime").get<long long>();
        } else {
            s.arriveTime = std::nullopt;
        }
    
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
    float extraversion; // -1 to 1

    friend void from_json(const nlohmann::json& j, TurtleData& t) {
        t.id = j.at("id").get<int>();
        t.position = j.at("position").get<Vec3>();
        t.dimension = j.at("dimension").get<string>();
        t.fuel = j.at("fuel").get<int>();
        t.busy = j.at("busy").get<bool>();
        t.face = j.at("face").get<neswDirections>();
        t.journeyPath = j.at("journeyPath").get<vector<JourneyStep>>();
        t.journeyStepIndex = j.at("journeyStepIndex").get<int>();
        t.extraversion = j.at("extraversion").get<float>();
    };
};

class Turtle {
    public:
        int id;
        websocketpp::connection_hdl ws;
        Vec3 position;
        string dimension;
        int fuel;
        bool busy;
        neswDirections face;
        vector<JourneyStep> journeyPath;
        int journeyStepIndex;

        float extraversion; // -1 to 1


        Turtle(websocketpp::connection_hdl ws_, const TurtleData& data);
        void Update(TurtleData data);
        std::optional<std::vector<JourneyStep>> GetJourneyPath(const std::vector<Vec3>& destinations);
};