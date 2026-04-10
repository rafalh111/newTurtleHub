// Turtle.cpp
#include <iostream>
#include <optional>
#include <chrono>
#include <queue>

#include "WorldMap.hpp"
#include "Turtle.hpp"
#include "Utils.hpp"

using namespace std;

extern unordered_map<string, WorldMap*> DimensionMaps;

Turtle::Turtle(websocketpp::connection_hdl ws_, const TurtleData& data) : 
    id(data.id), ws(ws_), position(data.position.x, data.position.y, data.position.z), dimension(data.dimension), fuel(data.fuel), busy(data.busy),
    face(data.face), journeyPath(data.journeyPath), journeyStepIndex(data.journeyStepIndex), extraversion(data.extraversion) {
}

void Turtle::Update(TurtleData data) {
    position = {data.position.x, data.position.y, data.position.z};
    dimension = data.dimension;
    fuel = data.fuel;
    busy = data.busy;
    face = data.face;
    journeyPath = data.journeyPath;
    journeyStepIndex = data.journeyStepIndex;
    extraversion = data.extraversion;
}

std::optional<std::vector<JourneyStep>> Turtle::GetJourneyPath(const std::vector<Vec3>& destinations) {
    auto isDestination = [&](Vec3& pos) -> bool {
        for (const Vec3& dest : destinations) {
            if (pos == dest) return true;
        };
        
        return false;
    };

    long long startTime = chrono::duration_cast<chrono::milliseconds>(
        chrono::system_clock::now().time_since_epoch()
    ).count();

    struct Node {
        Vec3 vector;
        std::optional<neswudDirections> neswudDirection;
        int fuelCostSoFar;
        long long arriveTime;
        std::vector<TurtleReservation> reservations;
        int pathsCrossed;
        int waitTime;
        neswDirections turtleFace;
        std::optional<frbludDirections> frbludDirection;
        int weight;
    };


    priority_queue<Node, vector<Node>, greater<Node>> queue; 

    queue.push( 
        Node {
            position, // vector
            std::nullopt, // neswudDirection
            0, // fuelCostSoFar
            startTime, // arriveTime
            DimensionMaps[dimension]->Get(position).reservations, // reservations
            0, // pathsCrossed
            0, // waitTime
            face, // turtleFace
            std::nullopt, // frbludDirection
            multiManhattanDistance(position, destinations), // weight (using Manhattan distance as a heuristic)
        }
    );

    unordered_map<Vec3, Node> cameFrom; // Example of using a visited map, replace with actual pathfinding logic
    unordered_map<Vec3, int> bestCost; // Example of using a cost map, replace with actual pathfinding logic
    
    while (!queue.empty()) {
        Node current = queue.top(); queue.pop();
        
        if (isDestination(current.vector)) {
            for (const TurtleReservation& reservation : current.reservations) {
                if (reservation.arriveTime >= current.arriveTime) {
                    return std::nullopt;
                }
            }

            // Reconstruct path
            vector<JourneyStep> path;
            Node node = current;

            while (cameFrom.find(node.vector) != cameFrom.end()) {
                JourneyStep step = {
                    {node.vector.x, node.vector.y, node.vector.z},
                    node.frbludDirection,
                    node.arriveTime,
                    node.waitTime
                };

                path.push_back(step);
                node = cameFrom[node.vector];
            }

            reverse(path.begin(), path.end());

            return path;
        }

        for (const Vec3& neighborVector : getNeighbors(current.vector)) {
            Vec3 neighborDirectionVector = neighborVector - current.vector;
            
            neswudDirections neighborDirection = duwsenDirectionVectors[neighborDirectionVector];
            
            int neighborFuelCostSoFar = current.fuelCostSoFar + 1;
            if (neighborFuelCostSoFar * 2 > fuel) {
                continue;
            }
            
            long long neighborArriveTime = current.arriveTime + 400;
            
            vector<TurtleReservation> neighborReservations = DimensionMaps[dimension]->Get(neighborVector).reservations;
            
            neswDirections neighborTurtleFace;
            if (neighborDirection == neswudDirections::up || neighborDirection == neswudDirections::down) {
                neighborTurtleFace = current.turtleFace;
            } else {
                neighborTurtleFace = static_cast<neswDirections>(neighborDirection);
            }
            
            frbludDirections neighborFrbludDirection = neswudToFrblud(
                static_cast<neswudDirections>(current.turtleFace), neighborDirection
            );
            
            if (neighborFrbludDirection == frbludDirections::right || neighborFrbludDirection == frbludDirections::left) {
                neighborArriveTime += 400;
            } else if (neighborFrbludDirection == frbludDirections::back) {
                neighborArriveTime += 800;
            }

            int neighborPathsCrossed = current.pathsCrossed;
            int neighborWaitTime = 0;
            bool permaBlocked;
            for (const auto& r : neighborReservations) {
                neighborPathsCrossed++;

                if (neighborArriveTime < r.arriveTime) {
                    continue;
                }

                if (!r.leaveTime.has_value()) {
                    permaBlocked = true;
                    break;
                }

                if (neighborArriveTime <= *r.leaveTime) {
                    int wait = *r.leaveTime - neighborArriveTime;
                    neighborArriveTime += wait;
                    neighborWaitTime += wait;
                }
            }

            if (permaBlocked == true) {
                continue;
            }
            
            long long neighborTimeCostSoFar = neighborArriveTime - current.arriveTime;
            int neighborWeight = neighborFuelCostSoFar*1 + neighborTimeCostSoFar*1 + neighborPathsCrossed*1 + multiManhattanDistance(neighborVector, destinations)*1;

            bool wontMakeItInTime = false;
            for (const TurtleReservation& currentReservation : current.reservations) {
                if (currentReservation.arriveTime < current.arriveTime) continue;
                if (currentReservation.arriveTime < neighborArriveTime) {
                    wontMakeItInTime = true;
                    break;
                }
            }

            if (wontMakeItInTime) {
                continue;
            }

            if (bestCost.find(neighborVector) != bestCost.end() && bestCost[neighborVector] <= neighborWeight) {
                continue;
            }
            
            bestCost[neighborVector] = neighborWeight;
            cameFrom[neighborVector] = current;
            queue.push(Node {
                neighborVector,
                neighborDirection,
                neighborFuelCostSoFar,
                neighborArriveTime,
                neighborReservations,
                neighborPathsCrossed,
                neighborWaitTime,
                neighborTurtleFace,
                neighborFrbludDirection,
                neighborWeight
            });
        }
    }

    return std::nullopt;
}