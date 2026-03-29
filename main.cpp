// main.cpp
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <memory>

#include "MessageHandlers.hpp"
#include "TurtleRegistry.hpp"
#include "Turtle.hpp"


using json = nlohmann::json;
using namespace std;

typedef websocketpp::server<websocketpp::config::asio> server;

TurtleRegistry registry;
server turtleHub;

unordered_map<string, function<void(const websocketpp::connection_hdl&, const json&)>> MessageTypes = {
    {"ping", Ping::handle},
    {"turtleBorn", TurtleBorn::handle}
};

int main() {
    turtleHub.init_asio();

    turtleHub.set_open_handler([](websocketpp::connection_hdl ws) {
        cout << "New connection established" << endl;
    });

    turtleHub.set_message_handler([](websocketpp::connection_hdl ws, server::message_ptr message) {
        try {
            json msg = json::parse(message->get_payload());
            if (!msg.contains("type") || !msg["type"].is_string()) {
                cout << "Message missing 'type' field or 'type' is not a string: " << msg.dump() << endl;
                return;
            }

            string type = msg["type"];

            if (MessageTypes.count(type)) {
                MessageTypes[type](ws, msg);
            } else {
                cout << "Unknown message type: " << type << endl;
            }
        } catch (const exception& e) {
            cout << "Invalid message: " << e.what() << endl;
        }
    });

    turtleHub.set_close_handler([](websocketpp::connection_hdl ws) {
        registry.unregisterByConnection(ws);
    });

    turtleHub.listen(9002);
    turtleHub.start_accept();

    cout << "TurtleHub server running on port 9002" << endl;

    turtleHub.run();

    return 0;
}