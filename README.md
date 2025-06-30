# Freefly Foam Dispenser

Greetings fellow engineer! This is a distributed drone control system for autonomous aerial 3D printing using the Freefly Alta X platform.

## Overview

This system transforms a Freefly Alta X drone into an autonomous aerial 3D printer capable of executing precise flight paths for large-scale foam structure construction. The architecture consists of a real-time C++ flight controller backend and a React-based operator console frontend, communicating via REST API over a local network.

## Architecture

### System Components

```
┌─────────────────┐    HTTP/REST API    ┌──────────────────┐
│   Frontend      │◄──────────────────► │    Backend       │
│   (React)       │                     │    (C++)         │
└─────────────────┘                     └──────────────────┘
                                                │
                                                │ MAVLink
                                                ▼
                                        ┌──────────────────┐
                                        │  PX4 SITL Gazebo │
                                        │   (Simulation)   │
                                        └──────────────────┘
```

### Backend (Flight Controller)
- **Language**: C++17
- **Responsibilities**:
  - MAVLink communication via MAVSDK
  - Mission waypoint parsing and execution
  - Real-time telemetry processing
  - HTTP API server for frontend communication
- **Dependencies**: MAVSDK, httplib.h

### Frontend (Operator Console)
- **Framework**: React
- **Responsibilities**:
  - Mission file upload and validation
  - Real-time flight monitoring
  - Interactive map visualization (OpenStreetMap)
  - Mission control interface (start/pause/abort)

## Quick Start

### Prerequisites

- C++17 compatible compiler
- CMake 3.10+
- [MAVSDK](https://mavsdk.mavlink.io/) library
- Node.js 16+ and npm
- PX4 SITL simulator

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/snithinbabunaidu/freefly_foam_dispenser/
   cd freefly-foam-dispenser
   ```

### Running the System

**1. Setting Up the Backend**

The backend executable must be running for the frontend to have something to connect to.

1. **Navigate to the Backend Directory:**

```
cd backend
```

2. **Configure the Build:**
   * Open the `CMakeLists.txt` file in a text editor.
   * **Crucially**, you must update the `MAVSDK_ROOT` variable to point to the absolute path of your MAVSDK library installation.

3. **Build the Project:** Create a build directory and run CMake and make.

```
mkdir build
cd build
cmake ..
make
```

4. **Run the Executable:** Once the build is complete, run the server.

```
./path_to_your_executable 
```

The backend server is now running and waiting for connections from the frontend.

**2. Setting Up the Frontend**

1. **Navigate to the Frontend Directory:**

```
cd frontend
```

2. **Install Dependencies:** This will download all the necessary libraries for the React application.

```
npm install
```

3. **Start the Application:** This command will launch a development server and open the operator console in your default web browser.

```
npm start
```

You should now see the Operator Console, ready to load a mission.

## Mission Planning

### Waypoint File Format

Mission waypoints are defined in a CSV format with the following specification:

```
latitude,longitude,relative_altitude_m
```

**Parameters:**
- `latitude`: Decimal degrees (WGS84)
- `longitude`: Decimal degrees (WGS84) 
- `relative_altitude_m`: Altitude in meters relative to takeoff position

**Example:**
```
47.397742,8.545594,10
47.397742,8.545794,10
47.397942,8.545794,10
47.397942,8.545594,10
47.397742,8.545594,10
```

### Mission Execution

1. Upload waypoint file via the operator console
2. Review planned path on the interactive map
3. Execute mission using the control panel:
   - **Start**: Begin mission execution
   - **Pause**: Suspend current mission
   - **Resume**: Continue paused mission
   - **Abort**: Immediately terminate and return to home

## API Reference

### Backend Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/start` | POST | Begin mission execution |
| `/pause` | POST | Pause current mission |
| `/resume` | POST | Resume paused mission |
| `/abort` | POST | Abort mission and RTL |
| `/telemetry` | GET | Retrieve current telemetry data |
| `/upload` | POST | Upload waypoint file |


## Project Structure

```
freefly_foam_dispenser/
├── .gitignore
├── backend/
│   ├── CMakeLists.txt
│   ├── httplib.h
│   └── test_conn.cpp
├── frontend/
│   ├── package.json
│   ├── public/
│   │   ├── index.html
│   │   └── manifest.json
│   └── src/
│       ├── App.css
│       ├── App.js
│       ├── OperatorConsole.js
│       ├── index.js
│       ├── components/
│       │   ├── Commands.js
│       │   ├── Map.js
│       │   └── Telemetry.js
│       └── services/
│           └── api.js
├── waypoints.txt
└── README.md
```

### Configuration

Before deploying to production hardware:

1. Update connection strings in backend configuration
2. Configure network settings for field deployment
3. Set appropriate failsafe parameters
4. Validate waypoint bounds checking

### Safety Considerations

- **Geofencing**: Implement virtual boundaries to prevent flights outside designated areas
- **Failsafe Modes**: Configure appropriate responses for communication loss
- **Battery Monitoring**: Set conservative voltage thresholds
- **Weather Limits**: Define operational wind speed and precipitation limits

## Future Enhancements

### Phase 1: Core Functionality - done
- Basic waypoint navigation
- Real-time telemetry
- Mission control interface

### Phase 2: Safety & Reliability
- Advanced failsafe mechanisms
- Redundant communication channels
- Flight envelope protection
- Automated pre-flight checks

### Phase 3: Advanced Features
- 3D mission visualization
- Foam dispenser integration
- Dynamic path optimization
- Multi-drone coordination

### Phase 4: Production Hardening
- Hardware-in-the-loop testing
- Field deployment tooling
- Performance optimization
