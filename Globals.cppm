//
// Created by efn on 7/1/26.
//

module;

#include <websocketpp/roles/server_endpoint.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
export module Globals;

import TurtleRegistry;
import WorldMap;

extern websocketpp::server<websocketpp::config::asio> turtleHub;
extern TurtleRegistry registry;
WorldMap OverworldMap;
WorldMap NetherMap;
WorldMap EndMap;

extern std::unordered_map<std::string, WorldMap*> DimensionMaps = {
    {"overworld", *OverworldMap},
    {"nether", *NetherMap},
    {"end", *EndMap}
};

// std::unordered_map<std::string, WorldMap*>& getDimensionMaps() {
//     static std::unordered_map<std::string, WorldMap*> maps = {
//         {"overworld", &OverworldMap},
//         {"nether", &NetherMap},
//         {"end", &EndMap}
//     };
//     return maps;
// }