// MessageHandlers.cpp
#pragma once
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <iostream>

#include "TurtleRegistry.hpp"
#include "Turtle.hpp"
#include "Utils.hpp"

using namespace std;

using json = nlohmann::json;

// Forward declare the registry and server
extern TurtleRegistry registry;
extern websocketpp::server<websocketpp::config::asio> turtleHub;

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
        int id;
        Vec3 position;
        bool busy;
        string face;
        vector<Step> journeyPath;
        int journeyStepIndex;

        friend void from_json(const json& j, Payload& p) {
            j.at("id").get_to(p.id);
            j.at("position").get_to(p.position);
            j.at("busy").get_to(p.busy);
            j.at("face").get_to(p.face);
            j.at("journeyPath").get_to(p.journeyPath);
            j.at("journeyStepIndex").get_to(p.journeyStepIndex);
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
    
            auto turtle = make_shared<Turtle>(
                pld.id,
                ws,
                pld.position,
                pld.busy,
                pld.face,
                pld.journeyPath,
                pld.journeyStepIndex
            );
    
            registry.registerTurtle(turtle);
        } catch (const exception& e) {
            cout << "Invalid turtleBorn message: " << e.what() << endl;
        }
    }
} // namespace TurtleBorn
