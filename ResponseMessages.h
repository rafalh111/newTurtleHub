//
// Created by efn on 5/5/26.
//

#ifndef UNTITLED_RESPONSEMESSAGES_H
#define UNTITLED_RESPONSEMESSAGES_H
#include <vector>
#include <optional>

#include "Utils.h"
#include "Turtle.h"

inline nlohmann::json getPingResponse(int number) {
    return nlohmann::json {
        {"type", "pong"},
        {"payload", {"number", number}},
    };
}

inline nlohmann::json getJourneyPathRequestResponse(std::optional<vector<JourneyStep>> journeyPath) {
    return nlohmann::json {
        {"type", "journeyPath"},
        {"payload", {"journeyPath", journeyPath}}
    };
}

inline nlohmann::json getJourneyStartResponse(bool permission) {
    return nlohmann::json {
        {"type", "journeyStartPermission"},
        {"payload", {"permission", permission}}
    };
}

inline nlohmann::json getObstacleWarningResponse(Vec3 vector) {
    return nlohmann::json {
        {"type", "ObstacleWarning"},
        {"payload", {"vector", vector}}
    };
}

#endif //UNTITLED_RESPONSEMESSAGES_H
