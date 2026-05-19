//
// Created by efn on 4/27/26.
//


// MessageHandlers.cpp
#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <optional>
#include <string>

#include "ResponseMessages.h"
#include "MessageHandlers.h"
#include "TurtleRegistry.h"
#include "WorldMap.h"
#include "Turtle.h"
#include "Utils.h"

using json = nlohmann::json;



using namespace std;

using json = nlohmann::json;

// Forward declare the registry and server
extern websocketpp::server<websocketpp::config::asio> turtleHub;
extern unordered_map<string, WorldMap*> DimensionMaps;
extern TurtleRegistry registry;

namespace Ping {
    struct Payload {
        int number;

        friend void from_json(const json& j, Payload& p) {
            j.at("number").get_to(p.number);
        }
    };

    struct Message {
        string type;
        Payload payload{};

        friend void from_json(const json& j, Message& m) {
            j.at("type").get_to(m.type);
            j.at("payload").get_to(m.payload);
        }
    };

    void handle(const websocketpp::connection_hdl& ws, const json& message) {
        try {
            auto [type, payload] = message.get<Message>();
            const json response = getPingResponse(payload.number);
            turtleHub.send(ws, response.dump(), websocketpp::frame::opcode::text);
        } catch (const exception& e) {
            cout << "Invalid ping message: " << e.what() << endl;
        }
    }
} // namespace Ping

namespace Heartbeat {
    struct Payload {
        TurtleData data;

        friend void from_json(const json& j, Payload& p) {
            j.at("data").get_to(p.data);
        }
    };

    struct Message {
        string type;
        Payload payload;

        friend void from_json(const json& j, Message& m) {
            j.at("type").get_to(m.type);
            j.at("payload").get_to(m.payload);
        }
    };

    void handle(const websocketpp::connection_hdl& ws, const json& message) {
        try {
            auto [type, payload] = message.get<Message>();
            auto&[data] = payload;

            if (registry.getByConnection(ws)) {
                registry.getByConnection(ws)->Update(data);
                return;
            }

            const auto turtle = make_shared<Turtle>(ws, data);
            registry.registerTurtle(turtle);
        } catch (const exception& e) {
            cout << "Invalid turtleBorn message: " << e.what() << endl;
        }
    }
} // namespace TurtleBorn

namespace JourneyPathRequest {
    struct Payload {
        TurtleData data;
        vector<Vec3> destinations;
        long long sendTime{};
        long long timeLimit{};
        long long timeAtTheEnd{};

        friend void from_json(const json& j, Payload& p) {
            j.at("data").get_to(p.data);
            j.at("destinations").get_to(p.destinations);
            j.at("sendTime").get_to(p.sendTime);
            j.at("timeLimit").get_to(p.timeLimit);
            j.at("timeAtTheEnd").get_to(p.timeAtTheEnd);
        }
    };

    struct Message {
        string type;
        Payload payload;

        friend void from_json(const json& j, Message& m) {
            j.at("type").get_to(m.type);
            j.at("payload").get_to(m.payload);
        }
    };

    void handle(const websocketpp::connection_hdl& ws, const json& message) {
        try {
            auto [type, payload] = message.get<Message>();

            if (!registry.getByConnection(ws)) {
                const auto turtle = make_shared<Turtle>(ws, payload.data);
                registry.registerTurtle(turtle);
            }

            const auto senderTurtle = registry.getByConnection(ws);
            senderTurtle->Update(payload.data);

            const auto& turtleMap =  DimensionMaps[senderTurtle->dimension];
            const std::optional<vector<JourneyStep>>  journeyPath = turtleMap->GetJourneyPath(
                senderTurtle, payload.destinations, payload.timeLimit, payload.timeAtTheEnd);

            const json response = getJourneyPathRequestResponse(journeyPath);
            turtleHub.send(ws, response.dump(), websocketpp::frame::opcode::text);
        } catch (const exception& e) {
            cout << "Invalid journey message: " << e.what() << endl;
        }
    }
}


namespace JourneyStart {
    struct Payload {
        TurtleData data;
        vector<JourneyStep> journeyPath;

        friend void from_json(const json& j, Payload& p) {
            j.at("data").get_to(p.data);
            j.at("journeyPath").get_to(p.journeyPath);
        }
    };

    struct Message {
        string type;
        Payload payload;

        friend void from_json(const json& j, Message& m) {
            j.at("type").get_to(m.type);
            j.at("payload").get_to(m.payload);
        }
    };

    void handle(const websocketpp::connection_hdl& ws, const json& message) {
        try {
            auto [type, payload] = message.get<Message>();
            auto&[data, journeyPath] = payload;

            if (!registry.getByConnection(ws)) {
                const auto turtle = make_shared<Turtle>(ws, data);
                registry.registerTurtle(turtle);
            }

            const auto senderTurtle = registry.getByConnection(ws);
            senderTurtle->Update(data);

            const bool permission = DimensionMaps[senderTurtle->dimension]->MakePathReservation(
                senderTurtle->id, payload.journeyPath);

            auto handlePositive = [&] {
                if (payload.journeyPath.empty()) return;

                WorldMap* map = DimensionMaps[senderTurtle->dimension];
                MapEntry* firstStepMapEntry = map->TryGet(senderTurtle->position);
                if (!firstStepMapEntry) return;

                auto& timeline = firstStepMapEntry->timeline;
                if (timeline.empty()) return;

                auto& entity = timeline.back();
                auto& [start, end] = entity->getTimeInterval();

                if (!end.has_value() && entity->hasId(senderTurtle->id)) {
                    end = payload.journeyPath[0].arriveTime;
                }
            };

            if (permission) handlePositive();

            const json response = getJourneyStartResponse(permission);
            turtleHub.send(ws, response.dump(), websocketpp::frame::opcode::text);
        } catch (const exception& e) {
            cout << "Invalid journey message: " << e.what() << endl;
        }
    }
}

