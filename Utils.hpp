// Utils.hpp
#include <nlohmann/json.hpp>
#include <functional>

using namespace std;

struct Vec3 {
    int x;
    int y;
    int z;

    bool operator==(const Vec3& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    string toString() const {
        return to_string(x) + "," + to_string(y) + "," + to_string(z);
    }

    static Vec3 fromString(const std::string& s) {
        Vec3 v;
        size_t p1 = s.find(',');
        size_t p2 = s.find(',', p1 + 1);

        if (p1 == std::string::npos || p2 == std::string::npos) {
            throw std::runtime_error("Invalid Vec3 string: " + s);
        }

        v.x = std::stoi(s.substr(0, p1));
        v.y = std::stoi(s.substr(p1 + 1, p2 - p1 - 1));
        v.z = std::stoi(s.substr(p2 + 1));

        return v;
    }

    friend void from_json(const nlohmann::json& j, Vec3& v) {
        v.x = j.at("x").get<int>();
        v.y = j.at("y").get<int>();
        v.z = j.at("z").get<int>();
    }
    
    friend void to_json(nlohmann::json& j, const Vec3& v) {
        j = nlohmann::json{{"x", v.x}, {"y", v.y}, {"z", v.z}};
    }
};

namespace std {
    template <>
    struct hash<Vec3> {
        size_t operator()(const Vec3& v) const {
            size_t h1 = hash<int>{}(v.x);
            size_t h2 = hash<int>{}(v.y);
            size_t h3 = hash<int>{}(v.z);

            // Combine hashes (simple but decent)
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
};


struct Step {
    Vec3 position;
    std::string action;

    friend void from_json(const nlohmann::json& j, Step& s) {
        s.position = j.at("position").get<Vec3>();
        s.action = j.at("action").get<std::string>();
    }
};

