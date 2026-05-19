//
// Created by efn on 4/27/26.
//

#ifndef UNTITLED_TURTLEREGISTRY_H
#define UNTITLED_TURTLEREGISTRY_H

// TurtleRegistry.hpp

#include <websocketpp/common/connection_hdl.hpp>
#include <unordered_map>
#include <memory>
#include <map>

#include "Turtle.h"
using namespace std;

class TurtleRegistry {
    map<websocketpp::connection_hdl,
    shared_ptr<Turtle>,
    owner_less<websocketpp::connection_hdl>> turtlesByConnection;
    unordered_map<int, shared_ptr<Turtle>> turtlesById;

    public:
        shared_ptr<Turtle> getByConnection(const websocketpp::connection_hdl& ws);
        shared_ptr<Turtle> getById(int id);

        void registerTurtle(const shared_ptr<Turtle>& turtle);

        void unregisterTurtle(const shared_ptr<Turtle>& turtle);
        void unregisterByConnection(const websocketpp::connection_hdl& ws);
        void unregisterById(int id);
};


#endif //UNTITLED_TURTLEREGISTRY_H
