// MessageHandlers.cpp
#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <iostream>

#include "TurtleRegistry.hpp"
#include "WorldMap.hpp"
#include "Turtle.hpp"
#include "Utils.hpp"

using namespace std;

using json = nlohmann::json;

// Forward declare the registry and server
extern TurtleRegistry registry;
extern websocketpp::server<websocketpp::config::asio> turtleHub;
extern unordered_map<string, WorldMap*> DimensionMaps;

namespace Ping {
    struct Message {
        string type;

        friend void from_json(const json& j, Message& m) {
            j.at("type").get_to(m.type);
        }
    };


    void handle(const websocketpp::connection_hdl& ws, const json& message) {
        try {
            json response = message;
            response["type"] = "pong";
    
            turtleHub.send(ws, response.dump(), websocketpp::frame::opcode::text);
        } catch (const exception& e) {
            cout << "Invalid ping message: " << e.what() << endl;
        }
    }
} // namespace Ping

namespace TurtleBorn {    
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
            Message msg = message.get<Message>();
            Payload& pld = msg.payload;

            if (registry.getByConnection(ws)) {
                registry.getByConnection(ws)->Update(pld.data);
                return;
            }
            
            auto turtle = make_shared<Turtle>(ws, pld.data);
            registry.registerTurtle(turtle);
        } catch (const exception& e) {
            cout << "Invalid turtleBorn message: " << e.what() << endl;
        }
    }
} // namespace TurtleBorn

namespace Journey {
    struct Payload {
        TurtleData data;
        vector<Vec3> destinations;
        long long sendTime;

        friend void from_json(const json& j, Payload& p) {
            j.at("data").get_to(p.data);
            j.at("destinations").get_to(p.destinations);
            j.at("sendTime").get_to(p.sendTime);
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
            Message msg = message.get<Message>();
            Payload& pld = msg.payload;
            
            if (!registry.getByConnection(ws)) {
                auto turtle = make_shared<Turtle>(ws, pld.data);
                registry.registerTurtle(turtle);
            }

            auto senderTurtle = registry.getByConnection(ws);
            senderTurtle->Update(pld.data);

            optional<vector<JourneyStep>> journeyPath = senderTurtle->GetJourneyPath(pld.destinations);

            if (journeyPath.has_value()) {
                for (auto& journeyStep : *journeyPath) {
                    DimensionMaps[senderTurtle->dimension]->MakeReservation(
                        senderTurtle->id,
                        {journeyStep.position.x, journeyStep.position.y, journeyStep.position.z},
                        journeyStep.arriveTime.value(),
                        journeyStep.leaveTime.value()
                    );
                }
            }

            json response = {
                {"type", "journeyPath"},
                {"payload", {"journeyPath", journeyPath}}
            };

            turtleHub.send(ws, response.dump(), websocketpp::frame::opcode::text);
        } catch (const exception& e) {
            cout << "Invalid journey message: " << e.what() << endl;
        }
    }
};
