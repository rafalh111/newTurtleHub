//
// Created by efn on 4/27/26.
//


// Turtle.cpp
#include <utility>
#include <websocketpp/common/connection_hdl.hpp>
#include <string>
#include <chrono>

#include "WorldMap.h"
#include "Turtle.h"
#include "Utils.h"

using namespace std;

Turtle::Turtle(websocketpp::connection_hdl ws_, const TurtleData& data) :
    id(data.id), ws(std::move(ws_)), position(data.position.x, data.position.y, data.position.z), dimension(data.dimension), fuel(data.fuel), busy(data.busy),
    face(data.face), journeyPath(data.journeyPath), journeyStepIndex(data.journeyStepIndex) {
}

void Turtle::Update(const TurtleData& data) {
    position = {data.position.x, data.position.y, data.position.z};
    dimension = data.dimension;
    fuel = data.fuel;
    busy = data.busy;
    face = data.face;
    journeyPath = data.journeyPath;
    journeyStepIndex = data.journeyStepIndex;
}