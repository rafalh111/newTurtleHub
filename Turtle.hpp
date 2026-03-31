// Turtle.hpp
#pragma once

#include <websocketpp/server.hpp>
#include <vector>
#include <string>

#include "Utils.hpp"
using namespace std;

class Turtle {
public:
    int id;
    websocketpp::connection_hdl ws;
    Vec3 position;
    string dimension;
    bool busy;
    string face;
    vector<Step> journeyPath;
    int journeyStepIndex;

    // Constructor declaration
    Turtle(
        int id_,
        websocketpp::connection_hdl ws_,
        const Vec3& position_,
        string dimension_,
        bool busy_,
        const string& face_,
        const vector<Step>& journeyPath_,
        int journeyStepIndex_
    );

    // Method declaration
    void update(const Vec3& position_,
        const string& dimension_,
        bool busy_,
        const string& face_,
        const vector<Step>& journeyPath_,
        int journeyStepIndex_
    );
};