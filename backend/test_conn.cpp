#include <iostream>
#include <thread>
#include <chrono>
#include <mavsdk/mavsdk.h>
#include <mavsdk/plugins/telemetry/telemetry.h>

int main() {
    mavsdk::Mavsdk mavsdk{mavsdk::Mavsdk::Configuration{mavsdk::ComponentType::GroundStation}};
    auto result = mavsdk.add_any_connection("udpin://0.0.0.0:14550");
    if (result != mavsdk::ConnectionResult::Success) {
        std::cout << "Connection failed: " << result << std::endl;
        return 1;
    }
    std::this_thread::sleep_for(std::chrono::seconds(15));
    auto systems = mavsdk.systems();
    if (systems.empty()) {
        std::cout << "no drone" << std::endl;
        return 1;
    }
    std::cout << "connected" << std::endl;
    return 0;
}