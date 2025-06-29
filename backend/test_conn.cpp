#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <future>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>
#include <mavsdk/plugins/mission/mission.h>
#include <mavsdk/plugins/action/action.h>
#include "httplib.h"

std::vector<mavsdk::Mission::MissionItem> read_waypoints(const std::string& filename) {
    std::vector<mavsdk::Mission::MissionItem> items;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return items;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string lat_str, lon_str, alt_str;
        if (std::getline(ss, lat_str, ',') && std::getline(ss, lon_str, ',') && std::getline(ss, alt_str, ',')) {
            try {
                double lat = std::stod(lat_str);
                double lon = std::stod(lon_str);
                float alt = std::stof(alt_str);
                mavsdk::Mission::MissionItem new_item{};
                new_item.latitude_deg = lat;
                new_item.longitude_deg = lon;
                new_item.relative_altitude_m = alt;
                new_item.is_fly_through = false;
                items.push_back(new_item);
            } catch (const std::exception& e) {
                std::cerr << "Warning: Skipping invalid line '" << line << "': " << e.what() << std::endl;
            }
        }
    }
    return items;
}

void wait_until_ready(std::shared_ptr<mavsdk::System> system) {
    auto telemetry = mavsdk::Telemetry{system};
    while (!telemetry.health_all_ok()) {
        std::cout << "Vehicle not ready. Waiting..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << "Vehicle ready to arm." << std::endl;
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <waypoint_file>" << std::endl;
        return 1;
    }
    auto mission_items = read_waypoints(argv[1]);
    if (mission_items.empty()) {
        std::cerr << "Error: No valid waypoints found." << std::endl;
        return 1;
    }
    std::cout << "successfully read " << mission_items.size() << " waypoints." << std::endl;
    mavsdk::Mavsdk mavsdk{mavsdk::Mavsdk::Configuration{mavsdk::ComponentType::GroundStation}};
    std::cout << "Connecting to drone simulator..." << std::endl;
    auto result = mavsdk.add_any_connection("udpin://0.0.0.0:14550");
    if (result != mavsdk::ConnectionResult::Success) {
        std::cerr << "connection failed: " << result << std::endl;
        return 1;
    }
    std::this_thread::sleep_for(std::chrono::seconds(5));
    auto system = mavsdk.systems().empty() ? nullptr : mavsdk.systems().at(0);
    if (!system) {
        std::cout << "drone not found" << std::endl;
        return 1;
    }
    std::cout << "successfully connected to a drone" << std::endl;
    wait_until_ready(system);
    auto mission = mavsdk::Mission{system};
    auto action = mavsdk::Action{system};
    httplib::Server svr;
    svr.Get("/hello", [](const httplib::Request &, httplib::Response &res) {
        res.set_content("Hello, World!", "text/plain");
    });

    svr.Get("/start", [&](const httplib::Request &, httplib::Response &res) {
        std::cout << "Received /start request. Uploading mission..." << std::endl;
        mavsdk::Mission::MissionPlan mission_plan{};
        mission_plan.mission_items = mission_items;
        mavsdk::Mission::Result upload_result = mission.upload_mission(mission_plan);
        if (upload_result != mavsdk::Mission::Result::Success) {
            std::cerr << "Mission upload failed: " << upload_result << std::endl;
            res.set_content("Mission upload failed!", "text/plain");
            return;
        }
        std::cout << "Mission uploaded. Arming..." << std::endl;
        mavsdk::Action::Result arm_result = action.arm();
        if (arm_result != mavsdk::Action::Result::Success) {
            std::cerr << "Arming failed: " << arm_result << std::endl;
            res.set_content("Arming failed!", "text/plain");
            return;
        }
        std::cout << "Armed. Starting mission..." << std::endl;
        mavsdk::Mission::Result start_result = mission.start_mission();
        if (start_result != mavsdk::Mission::Result::Success) {
            std::cerr << "Mission start failed: " << start_result << std::endl;
            res.set_content("Mission start failed!", "text/plain");
            return;
        }
        res.set_content("Mission started successfully!", "text/plain");
    });
    svr.Get("/pause", [&](const httplib::Request &, httplib::Response &res) {
        std::cout << "Received /pause request." << std::endl;
        mavsdk::Mission::Result pause_result = mission.pause_mission();
        if (pause_result != mavsdk::Mission::Result::Success) {
            std::cerr << "Failed to pause mission: " << pause_result << std::endl;
            res.set_content("Failed to pause mission!", "text/plain");
            return;
        }
        res.set_content("Mission paused.", "text/plain");
    });
    svr.Get("/abort", [&](const httplib::Request &, httplib::Response &res) {
        std::cout << "Received /abort request." << std::endl;
        mavsdk::Mission::Result clear_result = mission.clear_mission();
        if (clear_result != mavsdk::Mission::Result::Success) {
             std::cerr << "Failed to clear mission: " << clear_result << std::endl;
             res.set_content("Failed to abort mission!", "text/plain");
             return;
        }
        std::cout << "Mission aborted and cleared." << std::endl;
        res.set_content("Mission aborted.", "text/plain");
    });
    svr.Get("/resume", [&](const httplib::Request &, httplib::Response &res) {
        std::cout << "Received /resume request." << std::endl;
        mavsdk::Mission::Result resume_result = mission.start_mission();
        if (resume_result != mavsdk::Mission::Result::Success) {
            std::cerr << "Failed to resume mission: " << resume_result << std::endl;
            res.set_content("Failed to resume mission!", "text/plain");
            return;
        }
        res.set_content("Mission resumed.", "text/plain");
    });
    std::cout << "Starting REST API server on port 8080..." << std::endl;
    svr.listen("0.0.0.0", 8080);
    return 0;
}