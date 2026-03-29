// Utils.hpp
#include <nlohmann/json.hpp>

struct Vec3 {
    int x;
    int y;
    int z;
};

void from_json(const nlohmann::json& j, Vec3& v) {
    v.x = j.at("x").get<int>();
    v.y = j.at("y").get<int>();
    v.z = j.at("z").get<int>();
}

struct Step {
    Vec3 position;
    std::string action;
};

void from_json(const nlohmann::json& j, Step& s) {
    s.position = j.at("position").get<Vec3>();
    s.action = j.at("action").get<std::string>();
}
