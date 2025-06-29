#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <mavsdk/plugins/mission/mission.h>

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
    std::cout << "Successfully read " << mission_items.size() << " waypoints:" << std::endl;
    for (size_t i = 0; i < mission_items.size(); ++i) {
        std::cout << "Waypoint " << i + 1 << ": Lat=" << mission_items[i].latitude_deg
                  << ", Lon=" << mission_items[i].longitude_deg
                  << ", Alt=" << mission_items[i].relative_altitude_m << std::endl;
    }
    return 0;
}