module;
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

export module ResponseMessages;

import Utils;
export inline nlohmann::json getPingResponse(int number) {
    return nlohmann::json {
            {"type", "pong"},
            {"payload", {"number", number}},
        };
}

export inline nlohmann::json getJourneyPathRequestResponse(std::optional<std::vector<JourneyStep>> journeyPath) {
    return nlohmann::json {
            {"type", "journeyPath"},
            {"payload", {"journeyPath", journeyPath}}
    };
}

export inline nlohmann::json getJourneyStartResponse(bool permission) {
    return nlohmann::json {
            {"type", "journeyStartPermission"},
            {"payload", {"permission", permission}}
    };
}

export inline nlohmann::json getObstacleWarningResponse(Vec3 vector) {
    return nlohmann::json {
            {"type", "ObstacleWarning"},
            {"payload", {"vector", vector}}
    };
}