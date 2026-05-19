//
// Created by efn on 4/27/26.
//

#ifndef UNTITLED_WORLDMAP_H
#define UNTITLED_WORLDMAP_H

#include <unordered_map>
#include <vector>
#include <string>
#include <optional>
#include <utility>

#include "Turtle.h"
#include "Utils.h"

struct TimeInterval {
    long long start{};
    std::optional<long long> end;


    [[nodiscard]] bool OverlapsWith(const TimeInterval& other) const;
    [[nodiscard]] bool Encloses(const TimeInterval& inner) const;
    [[nodiscard]] bool EnclosesPoint(long long point) const;

    friend void to_json(nlohmann::json& j, const TimeInterval& t) {
        j = nlohmann::json{
            {"start", t.start},
            {"end", t.end.has_value() ? nlohmann::json(*t.end) : nullptr}
        };
    }
    friend void from_json(const nlohmann::json& j, TimeInterval& t) {
        t.start = j.at("start").get<long long>();

        if (j.contains("end") && !j.at("end").is_null()) {
            t.end = j.at("end").get<long long>();
        } else {
            t.end = std::nullopt;
        }
    }
};

struct Block;
struct TurtleReservation;

struct ITimelineEntity {
    virtual ~ITimelineEntity() = default;

    [[nodiscard]] virtual TimeInterval& getTimeInterval() = 0;
    virtual std::pair<bool, bool> handleSolidDetection(const Block& detectedBlock, Vec3 vector) = 0;
    virtual bool handleEmptyDetection() = 0;
    virtual bool hasId(int id) = 0;
};

struct Block : ITimelineEntity {
    TimeInterval timeInterval;
    std::string name;
    std::unordered_map<std::string, std::string> state;
    std::vector<std::string> tags;

    TimeInterval& getTimeInterval() override;
    std::pair<bool, bool> handleSolidDetection(const Block& detectedBlock, Vec3 vector) override;
    bool handleEmptyDetection() override;
    bool hasId(int id) override;

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
    bool hasId(int id) override;

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

class WorldMap {
    std::string mapFilePath;
    std::unordered_map<Vec3, MapEntry> map;
    void CleanEntry(const Vec3& pos);
    bool InsertTimelineEntity(Vec3 position,  std::unique_ptr<ITimelineEntity> entity, bool forced);

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

#endif //UNTITLED_WORLDMAP_H