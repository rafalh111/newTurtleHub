//
// Created by efn on 4/27/26.
//
// TurtleRegistry.cpp
#include <websocketpp/common/connection_hdl.hpp>
#include <iostream>

#include "TurtleRegistry.h"

using namespace std;

shared_ptr<Turtle> TurtleRegistry::getByConnection(const websocketpp::connection_hdl& ws) {
    const auto it = turtlesByConnection.find(ws);
    return it != turtlesByConnection.end() ? it->second : nullptr;
}

shared_ptr<Turtle> TurtleRegistry::getById(const int id) {
    const auto it = turtlesById.find(id);
    return it != turtlesById.end() ? it->second : nullptr;
}

void TurtleRegistry::registerTurtle(const shared_ptr<Turtle>& turtle) {
    // If ID already exists → remove old turtle first
    if (turtlesById.contains(turtle->id)) {
        const auto old = turtlesById[turtle->id];
        turtlesByConnection.erase(old->ws);
    }

    turtlesById[turtle->id] = turtle;
    turtlesByConnection[turtle->ws] = turtle;

    cout << "Registered turtle: " << turtle->id << endl;
}

void TurtleRegistry::unregisterTurtle(const shared_ptr<Turtle>& turtle) {
    turtlesById.erase(turtle->id);
    turtlesByConnection.erase(turtle->ws);
    cout << "Unregistered turtle: " << turtle->id << endl;
}

void TurtleRegistry::unregisterByConnection(const websocketpp::connection_hdl& ws) {
    if (const auto turtle = getByConnection(ws)) unregisterTurtle(turtle);
}

void TurtleRegistry::unregisterById(const int id) {
    if (const auto turtle = getById(id)) unregisterTurtle(turtle);
}