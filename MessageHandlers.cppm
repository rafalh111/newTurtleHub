//
// Created by efn on 7/2/26.
//

module;

#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>

#include <unordered_map>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

export module MessageHandlers;

import ResponseMessages;
import WorldMap;


import TurtleRegistry;
import Turtle;
import Utils;

using json = nlohmann::json;

// Forward declarations (same as your externs)
extern websocketpp::server<websocketpp::config::asio> turtleHub;
extern std::unordered_map<std::string, WorldMap*> DimensionMaps;
extern TurtleRegistry registry;

// -------------------- exported API --------------------

export namespace Ping {
    void handle(const websocketpp::connection_hdl&, const json&);
}

export namespace Heartbeat {
    void handle(const websocketpp::connection_hdl&, const json&);
}

export namespace JourneyPathRequest {
    void handle(const websocketpp::connection_hdl&, const json&);
}

export namespace JourneyStart {
    void handle(const websocketpp::connection_hdl&, const json&);
}

export inline std::unordered_map<
    std::string,
    std::function<void(const websocketpp::connection_hdl&, const json&)>
> MessageTypes = {
    {"ping", Ping::handle},
    {"turtleBorn", Heartbeat::handle},
    {"journeyPathRequest", JourneyPathRequest::handle},
    {"journeyStart", JourneyStart::handle}
};

// -------------------- implementation --------------------

namespace Ping {

struct Payload {
    int number;

    friend void from_json(const json& j, Payload& p) {
        j.at("number").get_to(p.number);
    }
};

struct Message {
    std::string type;
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
    } catch (const std::exception& e) {
        std::cout << "Invalid ping message: " << e.what() << std::endl;
    }
}

} // namespace Ping

// ------------------------------------------------------

namespace Heartbeat {
    struct Payload {
        TurtleData data;

        friend void from_json(const json& j, Payload& p) {
            j.at("data").get_to(p.data);
        }
    };

    struct Message {
        std::string type;
        Payload payload;

        friend void from_json(const json& j, Message& m) {
            j.at("type").get_to(m.type);
            j.at("payload").get_to(m.payload);
        }
    };

    void handle(const websocketpp::connection_hdl& ws, const json& message) {
        try {
            auto [type, payload] = message.get<Message>();
            if (const auto turtle = registry.getByConnection(ws)) {
                turtle->Update(payload.data);
                return;
            }

            const auto turtle = std::make_shared<Turtle>(ws, payload.data);
            registry.registerTurtle(turtle);
        } catch (const std::exception& e) {
            std::cout << "Invalid turtleBorn message: " << e.what() << std::endl;
        }
    }
} // namespace Heartbeat

// ------------------------------------------------------

namespace JourneyPathRequest {
    struct Payload {
        TurtleData data;
        std::vector<Vec3> destinations;
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
        std::string type;
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
                const auto turtle = std::make_shared<Turtle>(ws, payload.data);
                registry.registerTurtle(turtle);
            }

            const auto sender = registry.getByConnection(ws);
            if (!sender) throw std::invalid_argument("JourneyPathRequest message error: getByConnection failed");

            sender->Update(payload.data);
            auto path = sender->dimension->GetJourneyPath(
                sender,
                payload.destinations,
                payload.timeLimit,
                payload.timeAtTheEnd
            );

            const json response = getJourneyPathRequestResponse(path);
            turtleHub.send(ws, response.dump(), websocketpp::frame::opcode::text);

        } catch (const std::exception& e) {
            std::cout << "Invalid journey message: " << e.what() << std::endl;
        }
    }
} // namespace JourneyPathRequest

// ------------------------------------------------------

namespace JourneyStart {
    struct Payload {
        TurtleData data;
        std::vector<JourneyStep> journeyPath;

        friend void from_json(const json& j, Payload& p) {
            j.at("data").get_to(p.data);
            j.at("journeyPath").get_to(p.journeyPath);
        }
    };

    struct Message {
        std::string type;
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
                auto turtle = std::make_shared<Turtle>(ws, payload.data);
                registry.registerTurtle(turtle);
            }

            const auto sender = registry.getByConnection(ws);
            if (!sender) {
                std::cout<< "JourneyStart message error: getByConnection failed";
                return;
            }

            sender->Update(payload.data);

            const bool permission = sender->dimension->MakePathReservation(
                sender->id,
                payload.journeyPath
            );

            if (permission && !payload.journeyPath.empty()) {
                auto* entry = sender->dimension->TryGet(sender->position);

                if (entry && !entry->timeline.empty()) {
                    auto& entity = entry->timeline.back();
                    auto& [start, end] = entity->getTimeInterval();

                    if (!end.has_value() && entity->getId() == sender->id) {
                        end = payload.journeyPath[0].arriveTime;
                    }
                }
            }

            const json response = getJourneyStartResponse(permission);
            turtleHub.send(ws, response.dump(), websocketpp::frame::opcode::text);
            
        } catch (const std::exception& e) {
            std::cout << "Invalid journey message: " << e.what() << std::endl;
        }
    }
} // namespace JourneyStart