//
// Created by efn on 7/1/26.
//

module;

#include <websocketpp/roles/server_endpoint.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
export module Globals;

import TurtleRegistry;
import WorldMap;
import Utils;

export extern websocketpp::server<websocketpp::config::asio> turtleHub;
export extern TurtleRegistry registry;
WorldMap OverworldMap{"overworld.json"};
WorldMap NetherMap{"nether.json"};
WorldMap EndMap{"end.json"};

export std::unordered_map<std::string, WorldMap*> DimensionMaps = {
    {"overworld", &OverworldMap},
    {"nether", &NetherMap},
    {"end", &EndMap}
};

// std::unordered_map<std::string, WorldMap*>& getDimensionMaps() {
//     static std::unordered_map<std::string, WorldMap*> maps = {
//         {"overworld", &OverworldMap},
//         {"nether", &NetherMap},
//         {"end", &EndMap}
//     };
//     return maps;
// }