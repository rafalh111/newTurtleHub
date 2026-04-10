// Utils.hpp
#include <nlohmann/json.hpp>
#include <functional>
#include <stdexcept>
#include <optional>

using namespace std;

enum class neswDirections { north, east, south, west };
unordered_map<neswDirections, int> neswDirectionLookup = {
    {neswDirections::north, 0},
    {neswDirections::east, 1},
    {neswDirections::south, 2},
    {neswDirections::west, 3}
};

unordered_map<string, neswDirections> stringToNesw = {
    {"north", neswDirections::north},
    {"east", neswDirections::east},
    {"south", neswDirections::south},
    {"west", neswDirections::west}
};

enum class neswudDirections { north, east, south, west, up, down };
unordered_map<neswudDirections, int> neswudDirectionLookup = {
    {neswudDirections::north, 0},
    {neswudDirections::east, 1},
    {neswudDirections::south, 2},
    {neswudDirections::west, 3},
    {neswudDirections::up, 4},
    {neswudDirections::down, 5}
};

unordered_map<string, neswudDirections> stringToNeswud = {
    {"north", neswudDirections::north},
    {"east", neswudDirections::east},
    {"south", neswudDirections::south},
    {"west", neswudDirections::west},
    {"up", neswudDirections::up},
    {"down", neswudDirections::down}
};

enum class frbludDirections { forward, right, back, left, up, down };
unordered_map<frbludDirections, int> frbludDirectionLookup = {
    {frbludDirections::forward, 0},
    {frbludDirections::right, 1},
    {frbludDirections::back, 2},
    {frbludDirections::left, 3},
    {frbludDirections::up, 4},
    {frbludDirections::down, 5}
};

unordered_map<string, frbludDirections> stringToFrblud = {
    {"forward", frbludDirections::forward},
    {"right", frbludDirections::right},
    {"back", frbludDirections::back},
    {"left", frbludDirections::left},
    {"up", frbludDirections::up},
    {"down", frbludDirections::down}
};

frbludDirections neswudToFrblud(neswudDirections startNeswudDirection, neswudDirections endNeswudDirection) {
    int startIndex = neswudDirectionLookup[startNeswudDirection];
    int endIndex = neswudDirectionLookup[endNeswudDirection];
    if (startIndex == 4 || startIndex == 5) {
        return static_cast<frbludDirections>(endIndex); 
    }

    int diff = (endIndex - startIndex) % 4;
    switch (diff) {
        case 0:
            return frbludDirections::forward;
        case 1:
            return frbludDirections::right;
        case 2:
            return frbludDirections::back;
        case 3:
            return frbludDirections::left;
        default:
            throw runtime_error("Invalid direction conversion");
    }
}

unordered_map<neswudDirections, Vec3> neswudDirectionVectors = {
    {neswudDirections::north, {0, 0, -1}}, // north
    {neswudDirections::east, {1, 0, 0}}, // east
    {neswudDirections::south, {0, 0, 1}}, // south
    {neswudDirections::west, {-1, 0, 0}}, // west
    {neswudDirections::up, {0, 1, 0}}, // up
    {neswudDirections::down, {0, -1, 0}} // down
};

unordered_map<Vec3, neswudDirections> duwsenDirectionVectors = {
    {{0, 0, -1}, neswudDirections::north}, // north
    {{1, 0, 0}, neswudDirections::east}, // east
    {{0, 0, 1}, neswudDirections::south}, // south
    {{-1, 0, 0}, neswudDirections::west}, // west
    {{0, 1, 0}, neswudDirections::up}, // up
    {{0, -1, 0}, neswudDirections::down} // down
};

struct Vec3 {
    int x;
    int y;
    int z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(int x_, int y_, int z_) : x(x_), y(y_), z(z_) {}
    Vec3(const Vec3& other) : x(other.x), y(other.y), z(other.z) {}
    Vec3(Vec3&& other) = default;
    Vec3& operator=(const Vec3& other) = default;
    Vec3& operator=(Vec3&& other) = default;

    Vec3 operator+(const Vec3& other) const {
        return {x + other.x, y + other.y, z + other.z};
    }

    Vec3 operator-(const Vec3& other) const {
        return {x - other.x, y - other.y, z - other.z};
    }

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

vector<Vec3> getNeighbors(Vec3 v) {
    vector<Vec3> neighbors;
    for (const auto& [dir, vec] : neswudDirectionVectors) {
        neighbors.push_back({v.x + vec.x, v.y + vec.y, v.z + vec.z});
    }
    
    return neighbors;
}

int manhattanDistance(const Vec3& v1, const Vec3& v2) {
    return abs(v1.x - v2.x) + abs(v1.y - v2.y) + abs(v1.z - v2.z);
}

int multiManhattanDistance(const Vec3& v1, const vector<Vec3>& v2Vector) {
    if (v2Vector.empty()) {
        return 0;
    }

    int smallestDistance = manhattanDistance(v1, v2Vector[0]);
    for (const Vec3& v2 : v2Vector) {
        int distance = manhattanDistance(v1, v2);
        if (distance < smallestDistance) {
            smallestDistance = distance;
        }
    }
    return smallestDistance;
};
