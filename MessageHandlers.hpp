// MessageHandlers.hpp
#pragma once
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Ping {
    void handle(const websocketpp::connection_hdl&, const json&);
}

namespace TurtleBorn {
    void handle(const websocketpp::connection_hdl&, const json&);
}

namespace Journey {
    void handle(const websocketpp::connection_hdl&, const json&);
}