//
// Created by efn on 4/27/26.
//

// WorldMap.cpp
#include <websocketpp/common/connection_hdl.hpp>
#include <nlohmann/json.hpp>
#include <unordered_set>
#include <filesystem>
#include <iostream>
#include <utility>
#include <fstream>
#include <climits>
#include <string>

#include "WorldMap.h"

#include "Globals.h"
#include "Turtle.h"
#include "Utils.h"
#include "ResponseMessages.h"

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

bool Block::hasId(const int id) {
    return false;
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
    const websocketpp::connection_hdl ws = registry.getById(turtleId);
    const nlohmann::json response = getObstacleWarningResponse(vector);
    turtleHub.send(ws, response.dump(), websocketpp::frame::opcode::text);

    constexpr bool toBeRemoved = true;
    constexpr bool canInsert = true;

    return {toBeRemoved, canInsert};
}

bool TurtleReservation::hasId(const int id) {
    if (turtleId == id) {
        return true;
    }

    return false;
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

WorldMap::WorldMap(string mapFilePath_) : mapFilePath(std::move(mapFilePath_)) {
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

    for (int i = timeline.size() - 1; i >= 0; --i) {
        const auto& timelineEntity = timeline[i];
        auto [start, end] = timelineEntity->getTimeInterval();
        if (!end.has_value()) continue;
        if (now <= end.value()) continue;
        timeline.erase(timeline.begin() + i);
    }

    if (timeline.empty()) map.erase(pos);
}

MapEntry* WorldMap::TryGet(const Vec3& vector) {
    const auto it = map.find(vector);
    if (it == map.end()) return nullptr;
    return &it->second;
}

const MapEntry* WorldMap::TryGet(const Vec3& vector) const {
    const auto it = map.find(vector);
    if (it == map.end()) return nullptr;
    return &it->second;
}

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

bool WorldMap::InsertTimelineEntity(const Vec3 position, std::unique_ptr<ITimelineEntity> entity, const bool forced) {
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
        if (timeline.empty()) {
            map.erase(position);
        }
    }

    // final insertion point (recompute safe position after removals)
    const auto insertIt = std::lower_bound(
        timeline.begin(),
        timeline.end(),
        interval.start,
        [](const auto& existing, long long startTime) {
            return existing->getTimeInterval().start < startTime;
        }
    );

    timeline.emplace(insertIt, std::move(entity));
    return true;
}

bool WorldMap::MakeReservation(const int id, const Vec3 position, const TimeInterval& reservationTimeInterval, const bool forced) {
    TurtleReservation reservation;
    reservation.timeInterval = reservationTimeInterval;
    reservation.turtleId = id;

    // Insert new reservation (move for efficiency)
    return InsertTimelineEntity(position, std::make_unique<TurtleReservation>(reservation), forced);
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
    return InsertTimelineEntity(position, std::make_unique<Block>(block), forced);
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
        while (journeyPath[i + 1].position == journeyPath[i].position) {
            ++i;
        }

        timeInterval.end = journeyPath[i].leaveTime;
        pairs.emplace_back(journeyPath[i].position, timeInterval);
    }

    for (const auto& [position, timeInterval] : pairs) {
        if (MakeReservation(id, position, timeInterval, false) == false) return false;
    }

    return true;
}

std::optional<std::vector<JourneyStep> > WorldMap::GetJourneyPath(shared_ptr<Turtle> turtle,
                                                                  const std::vector<Vec3> &destinations,
                                                                  long long timeLimit, long long timeAtTheEnd) const {

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
        neswudDirections neswudDirection;
        int fuelCostSoFar;
        long long arriveTime;
        long long waitTime;
        neswDirections turtleFace;
        frbludDirections frbludDirection;
        long long weight;
        long long gCost;

        [[nodiscard]] StateKey MakeKey() const {
            return StateKey{
                vector,
                arriveTime,
                turtleFace
            };
        }

        JourneyStep MakeEdgeInfo() {
            return JourneyStep{
                {vector.x, vector.y, vector.z},
                frbludDirection,
                arriveTime,
                waitTime
            };
        }

        bool operator>(const Node& other) const {
            return weight > other.weight;
        }
    };
    struct paretoSet {
        long long time;
        long long cost;

        [[nodiscard]] bool Dominates(const paretoSet& other) const {
            return (time <= other.time && cost <= other.cost) &&
                   (time < other.time || cost < other.cost);
        }
    };

    std::unordered_map<StateKey, long long, StateKeyHasher> bestCost;

    std::priority_queue<Node, std::vector<Node>, std::greater<>> queue;
    std::unordered_map<StateKey, JourneyStep, StateKeyHasher> edgeInfo;
    std::unordered_map<StateKey, StateKey, StateKeyHasher> cameFrom;

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

        frbludDirections turnDirections[] = {
            frbludDirections::left,
            frbludDirections::right,
        };

        for (const auto turn_direction: turnDirections) {
            const Vec3 vector = current.vector;
            const int fuelCostSoFar = current.fuelCostSoFar;
            const long long arriveTime = current.arriveTime + turn_time;
            frbludDirections frbludDirection = turn_direction;
            neswDirections turtleFace = neswDirectionsArray[(neswDirectionLookup[current.turtleFace]
                                                    + frbludDirectionLookup[frbludDirection]) % 4];
            auto neswudDirection = static_cast<neswudDirections>(turtleFace);

            if (findColliding(vector, arriveTime)) continue;

            const long long timeSpent = arriveTime - startTime;
            if (timeSpent > timeLimit) continue;

            const long long gCost = timeSpent;
            const long long hCost = multiManhattanDistance(vector, destinations) * step_time; // 400ms for every step
            const long long weight = gCost + hCost;

            Node child = {
                vector,
                neswudDirection,
                fuelCostSoFar,
                arriveTime,
                0,
                turtleFace,
                frbludDirection,
                weight,
                gCost
            };


            children.push_back(child);
        }

        frbludDirections moveDirections[] = {
            frbludDirections::forward,
            frbludDirections::up,
            frbludDirections::down,
        };

        for (const auto move_direction : moveDirections) {
            const long long arriveTime = current.arriveTime + step_time;
            frbludDirections frbludDirection = move_direction;
            const int fuelCostSoFar = current.fuelCostSoFar + 1;
            if (fuelCostSoFar > turtle->fuel) {
                continue;
            }

            neswudDirections neswudDirection;
            const neswDirections turtleFace = current.turtleFace;

            if (move_direction == frbludDirections::forward) {
                neswudDirection = static_cast<neswudDirections>(current.turtleFace);
            } else {
                neswudDirection = static_cast<neswudDirections>(move_direction);
            }

            const Vec3 vector = current.vector + neswudDirectionVectors[neswudDirection];

            if (findColliding(vector, arriveTime)) continue;
            if (findColliding(current.vector, arriveTime)) continue;
             /* I treat the arrival in the child node and the departure from the parent as the same time for safety.
            If it doesn't make it to the child node before some other turtle gets in the spot, the child node is inaccessible */

            const long long timeSpent = arriveTime - startTime;
            if (timeSpent > timeLimit) continue;

            const long long gCost = timeSpent;
            const long long hCost = multiManhattanDistance(vector, destinations) * step_time; // 400ms for every step
            const long long weight = gCost + hCost;

            Node child = {
                vector,
                neswudDirection,
                fuelCostSoFar,
                arriveTime,
                0,
                turtleFace,
                frbludDirection,
                weight,
                gCost
            };

            children.push_back(child);
        }

        return children;
    };

    Node startNode{
        turtle->position,
        static_cast<neswudDirections>(turtle->face),
        0,
        startTime,
        0,
        turtle->face,
        frbludDirections::forward,
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
            auto key = StateKey{current.vector, current.arriveTime, current.turtleFace};

            while (key != StateKey{turtle->position, startTime, turtle->face}) {
                path.push_back(edgeInfo[key]);
                key = cameFrom[key];
            }

            ranges::reverse(path);
            return path;
        }

        for (vector<Node> children = getChildren(current); auto& child: children) {
            StateKey childKey = child.MakeKey();

            // pareto version
            // auto& paretoVector = paretoBest[childKey];
            // paretoSet candidate = {child.arriveTime, child.weight};
            //
            // if (paretoWorst(candidate, paretoVector)) continue;

            if (auto it = bestCost.find(childKey); it != bestCost.end() && it->second <= child.gCost) {
                continue;
            }

            edgeInfo[childKey] = child.MakeEdgeInfo();
            // paretoVector.push_back(candidate);
            bestCost[childKey] = child.gCost;
            cameFrom[childKey] = currentKey;
            queue.push(child);
        }
    }

    return std::nullopt;
}