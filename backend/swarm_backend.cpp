#include <iostream>
#include <string>
#include <vector>
#include <memory>

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
