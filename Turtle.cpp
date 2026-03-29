// Turtle.cpp
#include "Turtle.hpp"

Turtle::Turtle( int id_, websocketpp::connection_hdl ws_, const Vec3& position_, bool busy_, const std::string& face_, const std::vector<Step>& journeyPath_, int journeyStepIndex_)
    : id(id_), ws(ws_), position(position_), busy(busy_), face(face_), journeyPath(journeyPath_), journeyStepIndex(journeyStepIndex_) {

}

void Turtle::update(const Vec3& position_, bool busy_, const std::string& face_, const std::vector<Step>& journeyPath_, int journeyStepIndex_) {
    position = position_;
    busy = busy_;
    face = face_;
    journeyPath = journeyPath_;
    journeyStepIndex = journeyStepIndex_;
}