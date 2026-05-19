#ifndef UNTITLED_UTILS_H
#define UNTITLED_UTILS_H

#include <functional>
#include <stdexcept>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include <ranges>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#pragma once

enum class neswDirections { north, east, south, west };
enum class neswudDirections { north, east, south, west, up, down };
enum class frbludDirections { forward, right, back, left, up, down };

// Forward declare Vec3 for use in maps
struct Vec3;

inline neswDirections neswDirectionsArray[] = {
    neswDirections::north,
    neswDirections::east,
    neswDirections::south,
    neswDirections::west
};

inline neswudDirections neswudDirectionsArray[] = {
    neswudDirections::north,
    neswudDirections::east,
    neswudDirections::south,
    neswudDirections::west,
    neswudDirections::up,
    neswudDirections::down
};

inline frbludDirections frbludDirectionsArray[] = {
    frbludDirections::forward,
    frbludDirections::right,
    frbludDirections::back,
    frbludDirections::left,
    frbludDirections::up,
    frbludDirections::down
};

// Vec3 struct
struct Vec3 {
    int x;
    int y;
    int z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(const int x_, const int y_, const int z_) : x(x_), y(y_), z(z_) {}
    Vec3(const Vec3& other) = default;
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

    bool operator!=(const Vec3& other) const {
        return x != other.x || y != other.y || z != other.z;
    }

    [[nodiscard]] std::string toString() const {
        return std::to_string(x) + "," + std::to_string(y) + "," + std::to_string(z);
    }

    static Vec3 fromString(const std::string& s) {
        Vec3 v;
        const size_t p1 = s.find(',');
        const size_t p2 = s.find(',', p1 + 1);

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

template <>
struct std::hash<Vec3> {
    size_t operator()(const Vec3& v) const noexcept {
        const size_t h1 = std::hash<int>{}(v.x);
        const size_t h2 = std::hash<int>{}(v.y);
        const size_t h3 = std::hash<int>{}(v.z);
        return h1 ^ h2 << 1 ^ h3 << 2;
    }
};

// Direction lookups
inline std::unordered_map<neswDirections, int> neswDirectionLookup = {
    {neswDirections::north, 0},
    {neswDirections::east, 1},
    {neswDirections::south, 2},
    {neswDirections::west, 3}
};

inline std::unordered_map<std::string, neswDirections> stringToNesw = {
    {"north", neswDirections::north},
    {"east", neswDirections::east},
    {"south", neswDirections::south},
    {"west", neswDirections::west}
};

inline std::unordered_map<neswudDirections, int> neswudDirectionLookup = {
    {neswudDirections::north, 0},
    {neswudDirections::east, 1},
    {neswudDirections::south, 2},
    {neswudDirections::west, 3},
    {neswudDirections::up, 4},
    {neswudDirections::down, 5}
};

inline std::unordered_map<std::string, neswudDirections> stringToNeswud = {
    {"north", neswudDirections::north},
    {"east", neswudDirections::east},
    {"south", neswudDirections::south},
    {"west", neswudDirections::west},
    {"up", neswudDirections::up},
    {"down", neswudDirections::down}
};

inline std::unordered_map<frbludDirections, int> frbludDirectionLookup = {
    {frbludDirections::forward, 0},
    {frbludDirections::right, 1},
    {frbludDirections::back, 2},
    {frbludDirections::left, 3},
    {frbludDirections::up, 4},
    {frbludDirections::down, 5}
};

inline std::unordered_map<std::string, frbludDirections> stringToFrblud = {
    {"forward", frbludDirections::forward},
    {"right", frbludDirections::right},
    {"back", frbludDirections::back},
    {"left", frbludDirections::left},
    {"up", frbludDirections::up},
    {"down", frbludDirections::down}
};

inline frbludDirections neswudToFrblud(const neswudDirections startNeswudDirection,
                                        const neswudDirections endNeswudDirection) {
    const int startIndex = neswudDirectionLookup[startNeswudDirection];
    const int endIndex = neswudDirectionLookup[endNeswudDirection];

    if (startIndex == 4 || startIndex == 5) {
        return static_cast<frbludDirections>(endIndex);
    }

    switch ((endIndex - startIndex) % 4) {
        case 0:
            return frbludDirections::forward;
        case 1:
            return frbludDirections::right;
        case 2:
            return frbludDirections::back;
        case 3:
            return frbludDirections::left;
        default:
            throw std::runtime_error("Invalid direction conversion");
    }
}

inline std::unordered_map<neswudDirections, Vec3> neswudDirectionVectors = {
    {neswudDirections::north, Vec3(0, 0, -1)},
    {neswudDirections::east, Vec3(1, 0, 0)},
    {neswudDirections::south, Vec3(0, 0, 1)},
    {neswudDirections::west, Vec3(-1, 0, 0)},
    {neswudDirections::up, Vec3(0, 1, 0)},
    {neswudDirections::down, Vec3(0, -1, 0)}
};

inline std::unordered_map<Vec3, neswudDirections> duwsenDirectionVectors = {
    {Vec3(0, 0, -1), neswudDirections::north},
    {Vec3(1, 0, 0), neswudDirections::east},
    {Vec3(0, 0, 1), neswudDirections::south},
    {Vec3(-1, 0, 0), neswudDirections::west},
    {Vec3(0, 1, 0), neswudDirections::up},
    {Vec3(0, -1, 0), neswudDirections::down}
};

inline std::vector<Vec3> getNeighbors(const Vec3 v) {
    std::vector<Vec3> neighbors;

    for (const auto& vec : neswudDirectionVectors | std::views::values) {
        neighbors.emplace_back(v.x + vec.x, v.y + vec.y, v.z + vec.z);
    }

    return neighbors;
}

inline int manhattanDistance(const Vec3& v1, const Vec3& v2) {
    return std::abs(v1.x - v2.x) + std::abs(v1.y - v2.y) + std::abs(v1.z - v2.z);
}

inline int multiManhattanDistance(const Vec3& v1, const std::vector<Vec3>& v2Vector) {
    if (v2Vector.empty()) {
        return 0;
    }

    int smallestDistance = manhattanDistance(v1, v2Vector[0]);

    for (const Vec3& v2 : v2Vector) {
        if (const int distance = manhattanDistance(v1, v2); distance < smallestDistance) {
            smallestDistance = distance;
        }
    }

    return smallestDistance;
}

#endif //UNTITLED_UTILS_H