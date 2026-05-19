//
// Created by efn on 4/27/26.
//

#ifndef UNTITLED_MESSAGEHANDLERS_H
#define UNTITLED_MESSAGEHANDLERS_H

// MessageHandlers.hpp
#include <websocketpp/common/connection_hdl.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <functional>

using json = nlohmann::json;

namespace Ping {
    void handle(const websocketpp::connection_hdl&, const json&);
}

namespace Heartbeat {
    void handle(const websocketpp::connection_hdl&, const json&);
}

namespace JourneyPathRequest {
    void handle(const websocketpp::connection_hdl&, const json&);
}

namespace JourneyStart {
    void handle(const websocketpp::connection_hdl&, const json&);
}

inline std::unordered_map<std::string, std::function<void(const websocketpp::connection_hdl&, const json&)>> MessageTypes = {
    {"ping", Ping::handle},
    {"turtleBorn", Heartbeat::handle},
    {"journeyPathRequest", JourneyPathRequest::handle},
    {"journeyStart", JourneyStart::handle}
};

#endif //UNTITLED_MESSAGEHANDLERS_H
