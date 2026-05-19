//
// Created by efn on 5/4/26.
//

#ifndef UNTITLED_GLOBALS_H
#define UNTITLED_GLOBALS_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/roles/server_endpoint.hpp>
#include "WorldMap.h"
#include "TurtleRegistry.h"

extern websocketpp::server<websocketpp::config::asio> turtleHub;
extern std::unordered_map<std::string, WorldMap*> DimensionMaps;
extern WorldMap OverworldMap;
extern WorldMap NetherMap;
extern WorldMap EndMap;

extern TurtleRegistry registry;

#endif //UNTITLED_GLOBALS_H
