#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/action/action.h>
#include <mavsdk/plugins/mission/mission.h>
#include <mavsdk/plugins/telemetry/telemetry.h>

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
    }
};