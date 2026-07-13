//
// Created by efn on 7/1/26.
//

module;

#include <websocketpp/common/connection_hdl.hpp>
#include <unordered_map>
#include <iostream>
#include <memory>
#include <map>

export module TurtleRegistry;

import Turtle;

export class TurtleRegistry {
    std::map<websocketpp::connection_hdl,
    std::shared_ptr<Turtle>,
    std::owner_less<websocketpp::connection_hdl>> turtlesByConnection;
    std::unordered_map<int, std::shared_ptr<Turtle>> turtlesById;

    void unregisterTurtle(const std::shared_ptr<Turtle>& turtle);

    public:
        void registerTurtle(const std::shared_ptr<Turtle>& turtle);
        std::shared_ptr<Turtle> getByConnection(const websocketpp::connection_hdl& ws);
        std::shared_ptr<Turtle> getById(int id);
        void unregisterByConnection(const websocketpp::connection_hdl& ws);
        void unregisterById(int id);
};

std::shared_ptr<Turtle> TurtleRegistry::getByConnection(const websocketpp::connection_hdl& ws) {
    const auto it = turtlesByConnection.find(ws);
    return it != turtlesByConnection.end() ? it->second : nullptr;
}

std::shared_ptr<Turtle> TurtleRegistry::getById(const int id) {
    const auto it = turtlesById.find(id);
    return it != turtlesById.end() ? it->second : nullptr;
}

void TurtleRegistry::registerTurtle(const std::shared_ptr<Turtle>& turtle) {
    // If ID already exists → remove old turtle first
    if (turtlesById.contains(turtle->id)) {
        const auto old = turtlesById[turtle->id];
        turtlesByConnection.erase(old->ws);
    }

    turtlesById[turtle->id] = turtle;
    turtlesByConnection[turtle->ws] = turtle;

    std::cout << "Registered turtle: " << turtle->id << std::endl;
}

void TurtleRegistry::unregisterTurtle(const std::shared_ptr<Turtle>& turtle) {
    turtlesById.erase(turtle->id);
    turtlesByConnection.erase(turtle->ws);
    std::cout << "Unregistered turtle: " << turtle->id << std::endl;
}

void TurtleRegistry::unregisterByConnection(const websocketpp::connection_hdl& ws) {
    if (const auto turtle = getByConnection(ws)) unregisterTurtle(turtle);
}

void TurtleRegistry::unregisterById(const int id) {
    if (const auto turtle = getById(id)) unregisterTurtle(turtle);
}
