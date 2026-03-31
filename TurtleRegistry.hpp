// TurtleRegistry.hpp
#pragma once

#include <map>
#include <unordered_map>
#include <memory>
#include <functional>   // for owner_less
#include <websocketpp/server.hpp>

#include "Turtle.hpp"
using namespace std;

class TurtleRegistry {
    private:
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