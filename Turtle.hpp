// Turtle.hpp
#pragma once

#include <websocketpp/server.hpp>
#include <vector>
#include <string>

#include "Utils.hpp"

class Turtle {
public:
    int id;
    websocketpp::connection_hdl ws;
    Vec3 position;
    bool busy;
    std::string face;
    std::vector<Step> journeyPath;
    int journeyStepIndex;

    // Constructor declaration
    Turtle(int id_, websocketpp::connection_hdl ws_, const Vec3& position_, bool busy_, const std::string& face_, const std::vector<Step>& journeyPath_, int journeyStepIndex_);

    // Method declaration
    void update(const Vec3& position_, bool busy_, const std::string& face_, const std::vector<Step>& journeyPath_, int journeyStepIndex_);
};