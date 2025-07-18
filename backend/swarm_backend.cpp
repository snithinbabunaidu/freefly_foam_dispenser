#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/mission/mission.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include "httplib.h"

// Data struct for telemetry
struct Position {
    double latitude = 0.0;
    double longitude = 0.0;
};

struct MissionProgress {
    int current = 0;
    int total = 0;
};

struct Battery {
    float remaining_percent = 0.0f;
    float voltage_v = 0.0f;
};

struct Altitude {
    float relative_altitude_m = 0.0f;
    float sea_level_altitude_m = 0.0f;
};

struct Heading {
    double heading_deg = 0.0;
};

// Helper function to parse waypoints
std::vector<mavsdk::Mission::MissionItem> read_waypoints(std::istream& stream) {
    std::vector<mavsdk::Mission::MissionItem> items;
    std::string line;
    while (std::getline(stream, line)) {
        std::stringstream ss(line);
        std::string lat_str, lon_str, alt_str;
        if (std::getline(ss, lat_str, ',') && std::getline(ss, lon_str, ',') && std::getline(ss, alt_str, ',')) {
            try {
                mavsdk::Mission::MissionItem new_item{};
                new_item.latitude_deg = std::stod(lat_str);
                new_item.longitude_deg = std::stod(lon_str);
                new_item.relative_altitude_m = std::stof(alt_str);
                new_item.is_fly_through = false;
                items.push_back(new_item);
            } catch (const std::exception& e) {
                std::cerr << "Warning: Skipping invalid waypoint line '" << line << "': " << e.what() << std::endl;
            }
        }
    }
    return items;
}

class Drone {
public:
    // MAVSDK components
    std::shared_ptr<mavsdk::System> system;
    std::shared_ptr<mavsdk::Mission> mission;
    std::shared_ptr<mavsdk::Action> action;
    std::shared_ptr<mavsdk::Telemetry> telemetry;

    // telemetry data holders
    std::shared_ptr<Position> last_position;
    std::shared_ptr<MissionProgress> mission_progress;
    std::shared_ptr<Battery> battery_status;
    std::shared_ptr<Altitude> altitude;
    std::shared_ptr<Heading> heading;

    // constructor to initialize drone obj
    Drone(std::shared_ptr<mavsdk::System> sys) {
        system = sys;
        mission = std::make_shared<mavsdk::Mission>(system);
        action = std::make_shared<mavsdk::Action>(system);
        telemetry = std::make_shared<mavsdk::Telemetry>(system);

        last_position = std::make_shared<Position>();
        mission_progress = std::make_shared<MissionProgress>();
        battery_status = std::make_shared<Battery>();
        altitude = std::make_shared<Altitude>();
        heading = std::make_shared<Heading>();

        std::cout << "Drone object created. Subscribing to telemetry..." << std::endl;
        subscribe_to_telemetry();
    }

    // telemetry subscriptions
    void subscribe_to_telemetry() {
        telemetry->subscribe_position([this](mavsdk::Telemetry::Position p) {
            this->last_position->latitude = p.latitude_deg;
            this->last_position->longitude = p.longitude_deg;
        });

        mission->subscribe_mission_progress([this](mavsdk::Mission::MissionProgress mp) {
            this->mission_progress->current = mp.current;
            this->mission_progress->total = mp.total;
        });

        telemetry->subscribe_battery([this](mavsdk::Telemetry::Battery b) {
            this->battery_status->remaining_percent = b.remaining_percent;
            this->battery_status->voltage_v = b.voltage_v;
        });

        telemetry->subscribe_altitude([this](mavsdk::Telemetry::Altitude a) {
            this->altitude->relative_altitude_m = a.altitude_relative_m;
            this->altitude->sea_level_altitude_m = a.altitude_amsl_m;
        });

        telemetry->subscribe_heading([this](mavsdk::Telemetry::Heading h) {
            this->heading->heading_deg = h.heading_deg;
        });
    }
};

int main(int argc, char** argv) {
    mavsdk::Mavsdk mavsdk{mavsdk::Mavsdk::Configuration{mavsdk::ComponentType::GroundStation}};

    auto result = mavsdk.add_any_connection("udpin://0.0.0.0:14550");
    if (result != mavsdk::ConnectionResult::Success) {
        std::cerr << "Connection failed: " << result << std::endl;
        return 1;
    }
    std::cout << "Backend listening on UDP port 14550..." << std::endl;

    auto fleet = std::make_shared<std::vector<std::shared_ptr<Drone>>>();

    mavsdk.subscribe_on_new_system([&mavsdk, &fleet]() {
        auto new_system = mavsdk.systems().back();
        std::cout << "Discovered a new drone." << std::endl;
        fleet->push_back(std::make_shared<Drone>(new_system));
        std::cout << "Fleet size is now: " << fleet->size() << std::endl;
    });

    std::cout << "Waiting for the first drone to connect..." << std::endl;
    while (fleet->empty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "First drone has connected. Starting HTTP server on port 8080." << std::endl;

    httplib::Server svr;
    svr.set_pre_routing_handler([](const httplib::Request&, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        return httplib::Server::HandlerResponse::Unhandled;
    });
    svr.Options("/(.*)", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    svr.Post("/start", [&](const httplib::Request &req, httplib::Response &res) {
        auto drone = fleet->at(0);
        auto waypoints_body = req.body;

        std::thread([drone, waypoints_body]() {
            std::cout << "Waiting for drone to be ready..." << std::endl;
            while (!drone->telemetry->health_all_ok()) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
            std::cout << "Drone is ready." << std::endl;

            std::stringstream waypoint_stream(waypoints_body);
            auto mission_items = read_waypoints(waypoint_stream);
            mavsdk::Mission::MissionPlan mission_plan{};
            mission_plan.mission_items = mission_items;
            
            if (drone->mission->upload_mission(mission_plan) != mavsdk::Mission::Result::Success) { 
                std::cerr << "Mission upload failed" << std::endl; 
                return; 
            }
            if (drone->action->arm() != mavsdk::Action::Result::Success) { 
                std::cerr << "Arming failed" << std::endl; 
                return; 
            }
            if (drone->mission->start_mission() != mavsdk::Mission::Result::Success) { 
                std::cerr << "Mission start failed" << std::endl; 
                return; 
            }
            
            std::cout << "Mission started successfully in background." << std::endl;
        }).detach();

        res.set_content("Mission start command received.", "text/plain");
    });

    svr.Get("/pause", [&](const httplib::Request &, httplib::Response &res) {
        auto drone = fleet->at(0);
        if (drone->mission->pause_mission() != mavsdk::Mission::Result::Success) { res.status = 500; res.set_content("Pause failed", "text/plain"); return; }
        res.set_content("Mission paused", "text/plain");
    });

    svr.Get("/resume", [&](const httplib::Request &, httplib::Response &res) {
        auto drone = fleet->at(0);
        if (drone->mission->start_mission() != mavsdk::Mission::Result::Success) { res.status = 500; res.set_content("Resume failed", "text/plain"); return; }
        res.set_content("Mission resumed", "text/plain");
    });

    svr.Get("/abort", [&](const httplib::Request &, httplib::Response &res) {
        auto drone = fleet->at(0);
        if (drone->mission->clear_mission() != mavsdk::Mission::Result::Success) { res.status = 500; res.set_content("Clear mission failed", "text/plain"); return; }
        res.set_content("Mission aborted", "text/plain");
    });
    
    svr.Get("/telemetry", [&](const httplib::Request &, httplib::Response &res) {
        auto drone = fleet->at(0);
        std::string json = "{ \"latitude\": " + std::to_string(drone->last_position->latitude) +
                           ", \"longitude\": " + std::to_string(drone->last_position->longitude) + " }";
        res.set_content(json, "application/json");
    });

    svr.Get("/mission_progress", [&](const httplib::Request &, httplib::Response &res) {
        auto drone = fleet->at(0);
        std::string json = "{ \"current\": " + std::to_string(drone->mission_progress->current) +
                           ", \"total\": " + std::to_string(drone->mission_progress->total) + " }";
        res.set_content(json, "application/json");
    });

    svr.Get("/battery", [&](const httplib::Request &, httplib::Response &res) {
        auto drone = fleet->at(0);
        std::string json = "{ \"remaining_percent\": " + std::to_string(drone->battery_status->remaining_percent) +
                           ", \"voltage_v\": " + std::to_string(drone->battery_status->voltage_v) + " }";
        res.set_content(json, "application/json");
    });

    svr.Get("/altitude", [&](const httplib::Request &, httplib::Response &res) {
        auto drone = fleet->at(0);
        std::string json = "{ \"relative_altitude_m\": " + std::to_string(drone->altitude->relative_altitude_m) +
                           ", \"sea_level_altitude_m\": " + std::to_string(drone->altitude->sea_level_altitude_m) + " }";
        res.set_content(json, "application/json");
    });

    svr.Get("/heading", [&](const httplib::Request &, httplib::Response &res) {
        auto drone = fleet->at(0);
        std::string json = "{ \"heading_deg\": " + std::to_string(drone->heading->heading_deg) + " }";
        res.set_content(json, "application/json");
    });

    svr.listen("0.0.0.0", 8080);

    return 0;
}