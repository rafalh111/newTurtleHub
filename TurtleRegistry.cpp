// TurtleRegistry.cpp
#include "TurtleRegistry.hpp"
#include <iostream>

std::shared_ptr<Turtle> TurtleRegistry::getByConnection(const websocketpp::connection_hdl& ws) {
    auto it = turtlesByConnection.find(ws);
    return (it != turtlesByConnection.end()) ? it->second : nullptr;
}

std::shared_ptr<Turtle> TurtleRegistry::getById(int id) {
    auto it = turtlesById.find(id);
    return (it != turtlesById.end()) ? it->second : nullptr;
}

void TurtleRegistry::registerTurtle(const std::shared_ptr<Turtle>& turtle) {
    // If ID already exists → remove old turtle first
    if (turtlesById.count(turtle->id)) {
        auto old = turtlesById[turtle->id];
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
    auto turtle = getByConnection(ws);
    if (turtle) unregisterTurtle(turtle);
}

void TurtleRegistry::unregisterById(int id) {
    auto turtle = getById(id);
    if (turtle) unregisterTurtle(turtle);
}