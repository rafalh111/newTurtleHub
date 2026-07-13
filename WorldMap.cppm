//
// Created by efn on 7/2/26.
//

module;

#include <websocketpp/roles/server_endpoint.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <iostream>
#include <optional>
#include <utility>
#include <climits>
#include <fstream>
#include <vector>
#include <string>

export module WorldMap;

import Turtle;
import Utils;
import TurtleRegistry;
import ResponseMessages;

using namespace std;

extern websocketpp::server<websocketpp::config::asio> turtleHub;
extern TurtleRegistry registry;

struct Block;
struct TurtleReservation;

struct ITimelineEntity {
    virtual ~ITimelineEntity() = default;

    [[nodiscard]] virtual TimeInterval& getTimeInterval() = 0;
    virtual std::pair<bool, bool> handleSolidDetection(const Block& detectedBlock, Vec3 vector) = 0;
    virtual bool handleEmptyDetection() = 0;
    virtual std::optional<int> getId() = 0;
};

struct Block : ITimelineEntity {
    TimeInterval timeInterval;
    std::string name;
    std::unordered_map<std::string, std::string> state;
    std::vector<std::string> tags;

    TimeInterval& getTimeInterval() override;
    std::pair<bool, bool> handleSolidDetection(const Block& detectedBlock, Vec3 vector) override;
    bool handleEmptyDetection() override;
    std::optional<int> getId() override;

    friend void to_json(nlohmann::json& j, const Block& b) {
        j = nlohmann::json{
            {"timeInterval", b.timeInterval},
            {"name", b.name},
            {"state", b.state},
            {"tags", b.tags}
        };
    }
    friend void from_json(const nlohmann::json& j, Block& b) {
        b.timeInterval = j.at("timeInterval").get<TimeInterval>();
        b.name = j.at("name").get<std::string>();
        b.state = j.at("state").get<std::unordered_map<std::string, std::string>>();
        b.tags = j.at("tags").get<vector<std::string>>();
    }
};

struct TurtleReservation : ITimelineEntity {
    TimeInterval timeInterval;
    int turtleId{};

    [[nodiscard]] TimeInterval& getTimeInterval() override;
    std::pair<bool, bool> handleSolidDetection(const Block& detectedBlock, Vec3 vector) override;
    bool handleEmptyDetection() override;
    std::optional<int> getId() override;

    friend void to_json(nlohmann::json& j, const TurtleReservation& r) {
        j = nlohmann::json{
            {"timeInterval", r.timeInterval},
            {"turtleId", r.turtleId}
        };
    }
    friend void from_json(const nlohmann::json& j, TurtleReservation& r) {
        r.timeInterval = j.at("timeInterval").get<TimeInterval>();
        r.turtleId = j.at("turtleId").get<int>();
    }
};

struct MapEntry {
    vector<std::unique_ptr<ITimelineEntity>> timeline;

    ITimelineEntity* FindEntityByTime(long long time);

    friend void to_json(nlohmann::json& j, const MapEntry& m) {
        j["timeline"] = nlohmann::json::array();

        for (const auto& ptr : m.timeline) {
            nlohmann::json item;

            if (auto* b = dynamic_cast<Block*>(ptr.get())) {
                item = *b;
                item["type"] = "block";
            } else if (auto* r = dynamic_cast<TurtleReservation*>(ptr.get())) {
                item = *r;
                item["type"] = "reservation";
            } else {
                throw std::runtime_error("Unknown ITimelineEntity type");
            }

            j["timeline"].push_back(std::move(item));
        }
    }
    friend void from_json(const nlohmann::json& j, MapEntry& m) {
        m.timeline.clear();
        for (const auto& item : j.at("timeline")) {
            if (const std::string type = item.at("type").get<std::string>(); type == "block") {
                auto obj = std::make_unique<Block>();
                *obj = item.get<Block>();
                m.timeline.push_back(std::move(obj));
            } else if (type == "reservation") {
                auto obj = std::make_unique<TurtleReservation>();
                *obj = item.get<TurtleReservation>();
                m.timeline.push_back(std::move(obj));
            } else {
                throw std::runtime_error("Unknown timeline entity type: " + type);
            }
        }
    }
};

export class WorldMap {
    std::string mapFilePath;
    std::unordered_map<Vec3, MapEntry> map;
    void CleanEntry(const Vec3& pos);
    bool ValidateTimelineInsertion(Vec3 position, const std::unique_ptr<ITimelineEntity> &entity, bool forced);
    void InsertTimelineEntity(Vec3 position,  std::unique_ptr<ITimelineEntity> entity, bool recomputeInsertPosition = true);

    public:
        explicit WorldMap(std::string mapFilePath);
        MapEntry* TryGet(const Vec3& vector);
        const MapEntry* TryGet(const Vec3& vector) const;
        void SonarUpdate(Vec3 pos, const Block& block, bool solid);
        void Save();
        bool MakeReservation(int id, Vec3 position, const TimeInterval &reservationTimeInterval, bool forced);
        bool ScheduleBlock(const std::string &name, const std::unordered_map<std::string, std::string> &state,
                           const std::vector<std::string> &tags, Vec3 position, const TimeInterval &blockTimeInterval,
                           bool forced);

        bool MakePathReservation(int id, const std::vector<JourneyStep>& journeyPath);

        std::optional<std::vector<JourneyStep>> GetJourneyPath(shared_ptr<Turtle> turtle,
                                                                const std::vector<Vec3> &destinations,
                                                                long long timeLimit, long long timeAtTheEnd) const;
};

using namespace std;
using json = nlohmann::json;
namespace fs = filesystem;

bool TimeInterval::OverlapsWith(const TimeInterval& other) const {
    return start < other.end.value_or(LLONG_MAX) && end.value_or(LLONG_MAX) > other.start;
}

bool TimeInterval::Encloses(const TimeInterval& inner) const {
    return start < inner.start && inner.end.value_or(LLONG_MAX) < end.value_or(LLONG_MAX);
}

bool TimeInterval::EnclosesPoint(const long long point) const {
    return start <= point && point <= end;
}

TimeInterval& Block::getTimeInterval() {
    return timeInterval;
}

bool Block::handleEmptyDetection() {
    constexpr bool toBeRemoved = true;
    return toBeRemoved;
}

std::pair<bool, bool> Block::handleSolidDetection(const Block &detectedBlock, Vec3 vector) {
    constexpr bool toBeRemoved = false;
    constexpr bool canInsert = false;

    name = detectedBlock.name;
    state = detectedBlock.state;
    tags = detectedBlock.tags;

    return {toBeRemoved, canInsert};
}

std::optional<int> Block::getId() {
    return std::nullopt;
}

TimeInterval& TurtleReservation::getTimeInterval() {
    return timeInterval;
}

bool TurtleReservation::handleEmptyDetection() {
    constexpr bool toBeRemoved = false;
    return toBeRemoved;
}

std::pair<bool, bool> TurtleReservation::handleSolidDetection(const Block &detectedBlock, const Vec3 vector) {
    //critical situation, placeholder
    if (const auto turtlePtr = registry.getById(turtleId)) {
        turtlePtr->ObstacleWarning(vector);
    }

    constexpr bool toBeRemoved = true;
    constexpr bool canInsert = true;

    return {toBeRemoved, canInsert};
}

std::optional<int> TurtleReservation::getId() {
    return turtleId;
}

ITimelineEntity* MapEntry::FindEntityByTime(const long long time) {
    const auto it = std::upper_bound(
        timeline.begin(),
        timeline.end(),
        time,
        [](long long time_for_comparison, const auto& entity) {
            return time_for_comparison < entity->getTimeInterval().start;
        }
    );

    if (it == timeline.begin()) return nullptr;

    const auto curr = std::prev(it);
    const auto& timeline_entity = *curr;

    if (timeline_entity->getTimeInterval().EnclosesPoint(time)) {
        return timeline_entity.get();  // 👈 return raw pointer
    }

    return nullptr;
}

WorldMap::WorldMap(string mapFilePath) : mapFilePath(std::move(mapFilePath)) {
    if (!fs::exists(mapFilePath)) {
        cout << "Map file does not exist, starting with an empty map: " << mapFilePath << endl;
        return;
    }

    try {
        ifstream inputFile(mapFilePath);
        if (!inputFile.is_open()) {
            throw runtime_error("Failed to open map file: " + mapFilePath);
        }

        json j;
        inputFile >> j;

        for (auto& [stringKey, entryJson] : j.items()) {
            Vec3 pos = Vec3::fromString(stringKey);
            auto entry = entryJson.get<MapEntry>();
            map[pos] = entry;
        }

        inputFile.close();
    } catch (const exception& e) {
        cerr << "Error loading map from file: " << e.what() << endl;
    }
}

void WorldMap::CleanEntry(const Vec3& pos) {
    auto&[timeline] = map[pos];

    const long long now = chrono::duration_cast<chrono::milliseconds>(
        chrono::system_clock::now().time_since_epoch()
    ).count();

    auto isDangling = [&](const unique_ptr<ITimelineEntity> &timelineEntity) {
        if (!timelineEntity->getTimeInterval().EnclosesPoint(now)) return false;
        std::optional<int> optId = timelineEntity->getId();
        if (!optId.has_value()) return false;;
        if (registry.getById(optId.value())->position == pos) return false;
        return true;
    };

    auto isExpired = [&](const unique_ptr<ITimelineEntity> &timelineEntity) {
        if (timelineEntity->getTimeInterval().end.value_or(LONG_LONG_MAX) < now) {
            return true;
        }

        return false;
    };

    for (int i = timeline.size() - 1; i >= 0; --i) {
        if (const auto& timelineEntity = timeline[i];
            isDangling(timelineEntity) || isExpired(timelineEntity)) {
            timeline.erase(timeline.begin() + i);
        }
    }

    if (timeline.empty()) map.erase(pos);
}

MapEntry* WorldMap::TryGet(const Vec3& vector) {
    CleanEntry(vector);
    const auto it = map.find(vector);
    if (it == map.end()) return nullptr;

    return &it->second;
}

// const MapEntry* WorldMap::TryGet(const Vec3& vector) const {
//     const auto it = map.find(vector);
//     if (it == map.end()) return nullptr;
//
//     return &it->second;
// }

void WorldMap::Save() {
    try {
        nlohmann::json j;
        for (const auto& [pos, entry] : map) {
            j[pos.toString()] = entry;
        }

        ofstream outputFile(mapFilePath);
        if (!outputFile.is_open()) {
            throw runtime_error("Failed to open map file for saving: " + mapFilePath);
        }

        outputFile << j.dump(4) << endl;
        outputFile.close();
    } catch (const exception& e) {
        cerr << "Error saving map to file: " << e.what() << endl;
    }
}

bool WorldMap::ValidateTimelineInsertion(const Vec3 position, const std::unique_ptr<ITimelineEntity> &entity, const bool forced) {
    auto& timeline = map[position].timeline;
    const auto interval = entity->getTimeInterval();

    // First possible overlap position (by start time)
    auto it = std::lower_bound(
        timeline.begin(),
        timeline.end(),
        interval.start,
        [](const auto& existing, long long startTime) {
            return existing->getTimeInterval().start < startTime;
        }
    );

    // Scan forwards for overlaps
    while (it != timeline.end()) {
        if (const auto& existing = *it; !existing->getTimeInterval().OverlapsWith(interval)) {
            ++it;
            continue;
        }

        if (!forced) return false;
        it = timeline.erase(it);
        if (timeline.empty()) map.erase(position);
    }

    return true;
}

void WorldMap::InsertTimelineEntity(const Vec3 position, std::unique_ptr<ITimelineEntity> entity, bool recomputeInsertPosition) {
    auto& timeline = map[position].timeline;
    const auto [start, end] = entity->getTimeInterval();

    // final insertion point (recompute safe position after removals)
    const auto insertIt = std::lower_bound(
        timeline.begin(),
        timeline.end(),
        start,
        [](const auto& existing, long long startTime) {
            return existing->getTimeInterval().start < startTime;
        }
    );

    timeline.emplace(insertIt, std::move(entity));
}

bool WorldMap::MakeReservation(const int id, const Vec3 position,
                                const TimeInterval& reservationTimeInterval,
                                const bool forced) {
    TurtleReservation reservation;
    reservation.timeInterval = reservationTimeInterval;
    reservation.turtleId = id;

    std::unique_ptr<ITimelineEntity> ptr = std::make_unique<TurtleReservation>(reservation);

    if (!ValidateTimelineInsertion(position, ptr, forced)) return false;

    InsertTimelineEntity(position, std::move(ptr));
    return true;
}

bool WorldMap::ScheduleBlock(const std::string &name, const std::unordered_map<std::string, std::string> &state,
                             const std::vector<std::string> &tags, const Vec3 position,
                             const TimeInterval &blockTimeInterval, const bool forced) {
    Block block;
    block.timeInterval = blockTimeInterval;
    block.name = name;
    block.state = state;
    block.tags = tags;

    // Insert new reservation (move for efficiency)
    InsertTimelineEntity(position, std::make_unique<Block>(block));
    return true;
}

void WorldMap::SonarUpdate(const Vec3 pos, const Block& block, const bool solid) {
    auto [it, inserted] = map.try_emplace(pos);
    if (inserted) {
        it->second.timeline.emplace_back(std::make_unique<Block>(block));
        return;
    }

    auto&[timeline] = it->second;

    for (int i = static_cast<int>(timeline.size()) - 1; i >= 0; --i) {
        const auto& timeline_entity = timeline[i];
        if (!block.timeInterval.Encloses(timeline_entity->getTimeInterval())) {
            continue;
        }

        bool toBeRemoved;
        bool canInsert;
        if (solid) {
            auto [first, second] = timeline_entity->handleSolidDetection(block, pos);
            toBeRemoved = first;
            canInsert = second;
        } else {
            toBeRemoved = timeline_entity->handleEmptyDetection();
            canInsert = true;
        }

        if (toBeRemoved) {
            timeline.erase(timeline.begin() + i);
            if (timeline.empty()) {
                map.erase(pos);
            }
        }

        if (canInsert) break;
        return;
    }

    InsertTimelineEntity(pos, std::make_unique<Block>(block), false);
}

bool WorldMap::MakePathReservation(const int id, const vector<JourneyStep>& journeyPath) {
    std::vector<std::pair<Vec3, TimeInterval>> pairs;

    for (int i = 0; i < journeyPath.size(); ++i) {
        TimeInterval timeInterval;
        timeInterval.start = journeyPath[i].timeInterval.start;
        while (i + 1 < journeyPath.size() && journeyPath[i + 1].position == journeyPath[i].position) {
            ++i;
        }
        timeInterval.end = journeyPath[i].timeInterval.end;
        pairs.emplace_back(journeyPath[i].position, timeInterval);
    }

    for (const auto& [position, timeInterval] : pairs) {
        if (MakeReservation(id, position, timeInterval, false) == false) return false;
    }

    return true;
}

std::optional<std::vector<JourneyStep>> WorldMap::GetJourneyPath(shared_ptr<Turtle> turtle, const std::vector<Vec3> &destinations, long long timeLimit, long long timeAtTheEnd) const {
    std::unordered_set destinationsSet(destinations.begin(), destinations.end());
    long long startTime = chrono::duration_cast<chrono::milliseconds>(
        chrono::system_clock::now().time_since_epoch()
    ).count();

    long long step_time = 400;
    long long turn_time = 400;

    struct StateKey {
        Vec3 position;
        long long time{};
        neswDirections turtleFace{};

        bool operator==(const StateKey& other) const {
            return position == other.position &&
                   time == other.time &&
                   turtleFace == other.turtleFace;
        }

        bool operator!=(const StateKey& other) const {
            return position != other.position ||
                   time != other.time ||
                   turtleFace != other.turtleFace;
        }
    };
    struct StateKeyHasher {
        std::size_t operator()(const StateKey& k) const {
            const std::size_t h1 = std::hash<Vec3>()(k.position);
            const std::size_t h2 = std::hash<long long>()(k.time);
            const std::size_t h3 = std::hash<int>()(static_cast<int>(k.turtleFace));

            size_t seed = h1;
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
    struct Node {
        Vec3 vector;
        int fuelCostSoFar{};
        long long arriveTime{};
        neswDirections turtleFace{};
        TurtleAction action{};
        long long weight{};
        long long gCost{};

        [[nodiscard]] StateKey MakeKey() const {
            return StateKey{
                vector,
                arriveTime,
                turtleFace
            };
        }

        bool operator>(const Node& other) const {
            return weight > other.weight;
        }

        Node() = default;

        Node(Vec3 vec3, int i, long long start_time, neswudDirections face, TurtleAction forward, long long i1, int i2);
    };

    std::priority_queue<Node, std::vector<Node>, std::greater<>> queue;
    std::unordered_map<StateKey, long long, StateKeyHasher> bestCost;
    std::unordered_map<StateKey, StateKey, StateKeyHasher> cameFrom;
    std::unordered_map<StateKey, Node, StateKeyHasher> edgeInfo;

    auto findColliding = [&](const Vec3 vector, const long long time) -> ITimelineEntity* {
        const MapEntry* map_entry = TryGet(vector);
        if (!map_entry) return nullptr;

        auto& timeline = map_entry->timeline;

        const auto it = std::upper_bound(
            timeline.begin(),
            timeline.end(),
            time,
            [](long long time_for_comparison, const auto& entity) {
                return time_for_comparison < entity->getTimeInterval().start;
            }
        );

        if (it == timeline.begin()) return nullptr;

        const auto curr = std::prev(it);
        const auto& timeline_entity = *curr;

        if (timeline_entity->getTimeInterval().EnclosesPoint(time)) {
            return timeline_entity.get();  // 👈 return raw pointer
        }

        return nullptr;
    };

    auto getChildren = [&](const Node &current) -> vector<Node> {
        vector<Node> children;

        constexpr TurtleAction actions[] = {
            TurtleAction::Forward,
            TurtleAction::Up,
            TurtleAction::Down,
            TurtleAction::TurnLeft,
            TurtleAction::TurnRight,
        };

        for (const auto action : actions) {
            Node child = current;

            switch (action) {
                case TurtleAction::Forward:
                    child.arriveTime += step_time;
                    child.action = TurtleAction::Forward;
                    child.fuelCostSoFar += 1;
                    if (child.fuelCostSoFar > turtle->fuel) continue;
                    child.vector += neswudDirectionVectors[static_cast<neswudDirections>(current.turtleFace)];
                    if (findColliding(current.vector, child.arriveTime)) continue;
                    /* I treat the arrival in the child node and the departure from the parent as the same time for safety.
                    If it doesn't make it to the child node before some other turtle gets in the spot, the child node is inaccessible. */
                    break;
                case TurtleAction::Up:
                    child.arriveTime += step_time;
                    child.action = TurtleAction::Up;
                    child.fuelCostSoFar += 1;
                    if (child.fuelCostSoFar > turtle->fuel) continue;
                    child.vector += neswudDirectionVectors[neswudDirections::up];
                    if (findColliding(current.vector, child.arriveTime)) continue;
                    break;
                case TurtleAction::Down:
                    child.arriveTime += step_time;
                    child.action = TurtleAction::Down;
                    child.fuelCostSoFar += 1;
                    if (child.fuelCostSoFar > turtle->fuel) continue;
                    child.vector += neswudDirectionVectors[neswudDirections::down];
                    if (findColliding(current.vector, child.arriveTime)) continue;
                    break;
                case TurtleAction::TurnLeft:
                    child.arriveTime += turn_time;
                    child.turtleFace = neswDirectionsArray[(neswDirectionLookup[current.turtleFace] + 3) % 4];
                    break;
                case TurtleAction::TurnRight:
                    child.arriveTime += turn_time;
                    child.turtleFace = neswDirectionsArray[(neswDirectionLookup[current.turtleFace] + 1) % 4];
                    break;
            }

            if (findColliding(child.vector, child.arriveTime)) continue;

            const long long timeSpent = child.arriveTime - startTime;
            if (timeSpent > timeLimit) continue;

            child.gCost = timeSpent;
            const long long hCost = multiManhattanDistance(child.vector, destinations) * step_time; // 400ms for every step
            child.weight = child.gCost + hCost;

            children.push_back(child);
        }

        return children;
    };

    Node startNode {
        turtle->position,
        0,
        startTime,
        static_cast<neswudDirections>(turtle->face),
        TurtleAction::Forward,
        multiManhattanDistance(turtle->position, destinations) * step_time,
        0
    };

    bestCost[startNode.MakeKey()] = startNode.gCost;
    queue.push(startNode);

    while (!queue.empty()) {
        Node current = queue.top();
        StateKey currentKey = current.MakeKey();
        queue.pop();

        if (auto it = bestCost.find(currentKey); it != bestCost.end() && it->second < current.gCost) {
            continue;
        }

        if (destinationsSet.contains(current.vector)) {
            if (findColliding(current.vector, current.arriveTime)) return std::nullopt;
            if (findColliding(current.vector, current.arriveTime + timeAtTheEnd)) return std::nullopt;

            std::vector<JourneyStep> path;
            auto key = current.MakeKey();
            std::optional<long long> proceedingArrivalTime;
            while (key != StateKey{turtle->position, startTime, turtle->face}) {
                JourneyStep journeyStep{
                    edgeInfo[key].vector,
                    edgeInfo[key].action,
                    edgeInfo[key].arriveTime,
                    proceedingArrivalTime
                };
                path.push_back(journeyStep);
                proceedingArrivalTime = edgeInfo[key].arriveTime;
                key = cameFrom[key];
            }

            ranges::reverse(path);
            return path;
        }

        for (vector<Node> children = getChildren(current); auto& child: children) {
            StateKey childKey = child.MakeKey();
            if (auto it = bestCost.find(childKey); it != bestCost.end() && it->second <= child.gCost) {
                continue;
            }

            edgeInfo[childKey] = child;
            bestCost[childKey] = child.gCost;
            cameFrom[childKey] = currentKey;
            queue.push(child);
        }
    }

    return std::nullopt;
}