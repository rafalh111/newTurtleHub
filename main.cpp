// main.cpp
#include <websocketpp/common/connection_hdl.hpp>
#include <websocketpp/config/asio_no_tls.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <iostream>
#include <memory>
#include <string>

import MessageHandlers;
import TurtleRegistry;
import Globals;
import Turtle;
import WorldMap;

using json = nlohmann::json;
using namespace std;

typedef websocketpp::server<websocketpp::config::asio> server;

int getPort() {
    int port = 9002;
    std::string input;

    std::cout << "Enter port to listen on (default 9002): ";
    getline(std::cin, input);

    if (!input.empty()) {
        try {
            if (const int temp = stoi(input); temp >= 1 && temp <= 65535) {
                port = temp;
            } else {
                std::cout << "Port out of range. Using default.\n";
            }
        } catch (...) {
            std::cout << "Invalid input. Using default.\n";
        }
    }

    return port;
}

int main() {
    turtleHub.init_asio();

    turtleHub.set_open_handler([](const websocketpp::connection_hdl& ws) {
        std::cout << "New connection established" << endl;
    });

    turtleHub.set_message_handler([](const websocketpp::connection_hdl& ws, const server::message_ptr& message) {
        try {
            json msg = json::parse(message->get_payload());
            if (!msg.contains("type") || !msg["type"].is_string()) {
                throw std::runtime_error("Message missing 'type' or the type is not a string");
            }

            const string type = msg["type"];
            if (!MessageTypes.contains(type)) {
                throw std::runtime_error("Unknown message type: " + type);
            }

            MessageTypes[type](ws, msg);
        } catch (const exception& e) {
            cout << "Invalid message: " << e.what() << endl;
        }
    });

    turtleHub.set_close_handler([](const websocketpp::connection_hdl& ws) {
        shared_ptr<Turtle> turtle = registry.getByConnection(ws);

        WorldMap* map = DimensionMaps[turtle->dimension];
        MapEntry* firstStepMapEntry = map->TryGet(turtle->position);
        if (!firstStepMapEntry) return;

        auto& timeline = firstStepMapEntry->timeline;
        if (timeline.empty()) return;

        auto& entity = timeline.back();
        auto& [start, end] = entity->getTimeInterval();

        if (!end.has_value() && entity->hasId(turtle->id)) {
            long long now = chrono::duration_cast<chrono::milliseconds>(
                chrono::system_clock::now().time_since_epoch()
            ).count();

            end = now;
        }

        registry.unregisterByConnection(ws);
    });


    const int port = getPort();
    turtleHub.listen(port);
    turtleHub.start_accept();

    cout << "TurtleHub server running on port " << port << endl;

    turtleHub.run();

    return 0;
}