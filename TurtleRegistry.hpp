// TurtleRegistry.hpp
#pragma once

#include <map>
#include <unordered_map>
#include <memory>
#include <functional>   // for owner_less
#include <websocketpp/server.hpp>

#include "Turtle.hpp"

class TurtleRegistry {
private:
    std::map<websocketpp::connection_hdl, std::shared_ptr<Turtle>, std::owner_less<websocketpp::connection_hdl>> turtlesByConnection;
    std::unordered_map<int, std::shared_ptr<Turtle>> turtlesById;

public:
    std::shared_ptr<Turtle> getByConnection(const websocketpp::connection_hdl& ws);
    std::shared_ptr<Turtle> getById(int id);

    void registerTurtle(const std::shared_ptr<Turtle>& turtle);
    void unregisterTurtle(const std::shared_ptr<Turtle>& turtle);
    void unregisterByConnection(const websocketpp::connection_hdl& ws);
    void unregisterById(int id);
};